// Mid-floor save test (6c-2b). NOT part of the game build (tasks.json compiles main.cpp and
// src/*.cpp only); it builds its own worlds in memory and writes its saves next to itself, never
// to app/save.txt. From the repo root, with build/ (ignored by git) as the scratch directory:
//
//   mkdir -p build/save_midfloor && cp -r app/resources build/save_midfloor/
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -I app/src app/tests/save_midfloor.cpp app/src/*.cpp -o build/save_midfloor/save_midfloor
//   (cd build/save_midfloor && ./save_midfloor)
//
// Exit 0 and "PASS" is the pass.
//
// The rule: closing the game in the middle of a floor must change nothing. A sortie stopped after
// any number of advanceSortie calls, written with saveGame, read back with loadGame into a fresh
// world and played on to the end must print the same chronicle, character for character, and
// leave the same world behind as the same sortie played without stopping. "The same world" is
// compared as the text of a final saveGame from each side, which is everything the game keeps.
//
// Two kinds of split, told apart in the report:
//   - control: stopped before the party enters (no floor state yet). This already worked before
//     6c-2b; if it fails, the save loses something other than the floor, and the floor cases
//     cannot be trusted until that is fixed.
//   - mid-floor: stopped after entering, with the floor still Ongoing. This is what 6c-2b adds:
//     until the floor state and the rng are saved, the reloaded party walks onto the floor again.
//
// The grid follows equivalence.cpp (same world builder), smaller: every mission, three
// difficulties around parity, eight seeds, and every split point of every sortie.
#include "Incursion.h"
#include "Generator.h"
#include "SaveGame.h"
#include "Utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

static const char *typeName(ObjectiveType t)
{
    switch (t)
    {
    case ObjectiveType::Slay:     return "Slay";
    case ObjectiveType::Hold:     return "Hold";
    case ObjectiveType::Retrieve: return "Retrieve";
    case ObjectiveType::Rescue:   return "Rescue";
    }
    return "?";
}

struct Case
{
    ObjectiveType type;
    int percent;
    int seed;
};

// One world, built the same way every time for the same case: a party of 1-4 generated units,
// sometimes hurt, sometimes greedy, on one floor with its objective set, and the sortie already
// in state.sorties -- where saveGame looks for it.
static void buildWorld(const Case &c, GameState &state, Roster &roster, Barracks &barracks)
{
    std::mt19937 genRng(static_cast<unsigned int>(c.seed * 7919 + c.percent));
    Generator gen("resources", genRng);
    barracks.create("A");
    const int partySize = 1 + c.seed % 4;
    for (int i = 0; i < partySize; ++i)
    {
        int id = state.nextUnitId++;
        roster.addUnit(gen.generateUnit(id, state.necropolis, SUMMON_TIERS[c.seed % 3]));
        barracks.assign(id, 0, roster);
        Unit &u = roster.findUnitById(id);
        if (c.seed % 4 == 1)
            u.takeDamage(u.getStats().getMaxHealth() * 3 / 4);
        if (c.seed % 5 == 0)
        {
            u.addSkill("Greedy");
            u.addSkill("Curious");
        }
    }

    int expected = 0;
    for (int id : barracks.at(0).getMembersIds())
    {
        const Unit &u = roster.findUnitById(id);
        expected += u.getStats().getStrength() + u.getStats().getConstitution()
                    + traitFit(c.type, u.getSkills());
    }
    const int difficulty = std::max(10, expected * c.percent / 100);
    const int floors[] = {2, 6, 14};
    const int floor = floors[(c.seed / 3) % 3];
    const int rounds = 3 + c.seed % 3;
    const bool counted = c.type == ObjectiveType::Hold || c.type == ObjectiveType::Rescue;
    state.floorObjectives[floor] = Objective{c.type, difficulty, counted ? rounds : 0};

    Sortie sortie;
    sortie.partyIndex = 0;
    sortie.floor = floor;
    sortie.departedAt = 0;
    sortie.seed = static_cast<unsigned int>(c.seed * 104729 + difficulty * 31 + static_cast<int>(c.type));
    state.sorties.push_back(sortie);
}

static std::string readFile(const std::string &path)
{
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// The chronicle and the final world of one run. `calls` counts every advanceSortie made, the
// entering one included, so a run with calls == N had N - 1 places it could have been stopped.
struct Run
{
    std::string chronicle;
    std::string finalSave;
    int calls = 0;
};

// Plays the case to the end. If stopAfter >= 0, after that many calls the world is saved, thrown
// away, and loaded into a fresh one that plays on; stopAfter == 0 saves before the party enters.
static Run play(const Case &c, int stopAfter,
                const std::vector<Encounter> &encounters, const std::vector<Injury> &injuries)
{
    Run run;
    std::streambuf *old = std::cout.rdbuf();
    std::ostringstream sink;
    std::cout.rdbuf(sink.rdbuf());

    GameState state;
    Roster roster;
    Barracks barracks;
    buildWorld(c, state, roster, barracks);
    std::vector<Postings::Moved> moved;
    Postings postings = Postings::fromSave(std::move(barracks), TrainingCamp(), state, moved);

    StepResult result = StepResult::Ongoing;
    for (;;)
    {
        if (run.calls == stopAfter)
        {
            saveGame("midfloor.txt", state, roster, postings.barracks(), postings.camp());

            GameState loadedState;
            Roster loadedRoster;
            Barracks loadedBarracks;
            TrainingCamp loadedCamp;
            loadGame("midfloor.txt", loadedState, loadedRoster, loadedBarracks, loadedCamp);
            std::vector<Postings::Moved> loadedMoved;
            Postings loadedPostings = Postings::fromSave(std::move(loadedBarracks), std::move(loadedCamp),
                                                         loadedState, loadedMoved);
            state = std::move(loadedState);
            roster = std::move(loadedRoster);
            postings = std::move(loadedPostings);
        }
        if (result != StepResult::Ongoing)
            break;
        // A cap well above any real floor, so a save that loses the step count cannot hang the test.
        if (run.calls > 500)
        {
            sink << "\n!!! more than 500 calls, the floor never ended\n";
            break;
        }
        result = advanceSortie(state.sorties.at(0), postings.party(0), roster, state, encounters, injuries);
        ++run.calls;
    }

    saveGame("final.txt", state, roster, postings.barracks(), postings.camp());
    run.finalSave = readFile("final.txt");

    std::cout.rdbuf(old);
    run.chronicle = sink.str();
    return run;
}

// The first line where two texts part, for the report.
static std::string firstDifference(const std::string &a, const std::string &b)
{
    std::istringstream sa(a), sb(b);
    std::string la, lb;
    int n = 1;
    for (;; ++n)
    {
        bool ha = static_cast<bool>(std::getline(sa, la));
        bool hb = static_cast<bool>(std::getline(sb, lb));
        if (!ha && !hb)
            return "(no line differs)";
        if (!ha) la = "<end>";
        if (!hb) lb = "<end>";
        if (!ha || !hb || la != lb)
            return "line " + std::to_string(n) + "\n      uninterrupted: " + la + "\n      reloaded:      " + lb;
    }
}

int main()
{
    const std::vector<Encounter> encounters = loadEncounters("resources/encounters.txt");
    const std::vector<Injury> injuries = loadInjuries("resources/injuries.txt");

    const ObjectiveType types[] = {ObjectiveType::Slay, ObjectiveType::Hold,
                                   ObjectiveType::Retrieve, ObjectiveType::Rescue};
    const int percents[] = {85, 100, 115};
    const int SEEDS = 8;

    int controls = 0, controlsBroken = 0;
    int splits = 0, splitsBroken = 0;
    bool reported = false;

    for (ObjectiveType type : types)
        for (int percent : percents)
            for (int seed = 1; seed <= SEEDS; ++seed)
            {
                const Case c{type, percent, seed};
                const Run whole = play(c, -1, encounters, injuries);

                // 0 is the control; 1 .. calls-1 stop with the party on the floor and the floor
                // still going (the last call is the one that ends it, so nothing is left after it).
                for (int stop = 0; stop < whole.calls; ++stop)
                {
                    const Run split = play(c, stop, encounters, injuries);
                    const bool sameChronicle = split.chronicle == whole.chronicle;
                    const bool sameWorld = split.finalSave == whole.finalSave;
                    const bool ok = sameChronicle && sameWorld;

                    if (stop == 0) { ++controls; if (!ok) ++controlsBroken; }
                    else           { ++splits;   if (!ok) ++splitsBroken; }

                    if (!ok && !reported)
                    {
                        std::cout << "first broken case: " << typeName(type) << " " << percent << "% seed " << seed
                                  << ", stopped after " << stop << " of " << whole.calls << " calls"
                                  << (stop == 0 ? " (control)" : "") << std::endl;
                        if (!sameChronicle)
                            std::cout << "  chronicle differs at " << firstDifference(whole.chronicle, split.chronicle)
                                      << std::endl;
                        if (!sameWorld)
                            std::cout << "  final world differs at " << firstDifference(whole.finalSave, split.finalSave)
                                      << std::endl;
                        reported = true;
                    }
                }
            }

    std::cout << "controls (saved before entering): " << controls - controlsBroken << " of " << controls
              << " match" << std::endl;
    std::cout << "mid-floor (saved with the floor going): " << splits - splitsBroken << " of " << splits
              << " match" << std::endl;

    if (controlsBroken > 0)
    {
        std::cout << "FAIL: the save loses something besides the floor; fix that before trusting the rest"
                  << std::endl;
        return 1;
    }
    if (splits == 0)
    {
        std::cout << "FAIL: no sortie was ever stopped mid-floor, the case is not exercised" << std::endl;
        return 1;
    }
    if (splitsBroken > 0)
    {
        std::cout << "FAIL" << std::endl;
        return 1;
    }
    std::cout << "PASS" << std::endl;
    return 0;
}
