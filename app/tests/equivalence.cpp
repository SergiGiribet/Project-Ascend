// Resolution equivalence harness. NOT part of the game build (tasks.json compiles main.cpp and
// src/*.cpp only); run it through tests/equivalence.sh, never on its own.
//
// What it is for: proving that a refactor of the resolution changed NOTHING the player can see or
// the game can remember. It plays a fixed grid of sorties -- every mission type, a spread of
// difficulties, many seeds -- each on a freshly built world, and prints everything: the chronicle
// the console shows, and then the state left behind (every unit's health, level, experience,
// traits and injuries; essence; the record; the dead). The script compiles it once against a
// baseline commit and once against the working tree and diffs the two outputs. Identical output,
// character for character, is the pass.
//
// It only uses the public API (resolveSortie, Sortie, Generator, Roster, Barracks, GameState), so
// the same file compiles against old and new sources as long as that API holds. If a refactor has
// to change it, this file changes in the same commit and the baseline is re-read against it.
#include "Incursion.h"
#include "Generator.h"
#include "Utils.h"

#include <algorithm>
#include <iostream>

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

static void printState(const Roster &roster, const GameState &state)
{
    std::cout << "--- state: essence " << state.essence << ", record " << state.highestFloor
              << ", dead " << state.necropolis.records().size() << std::endl;
    for (const Unit &u : roster.units())
    {
        std::cout << "  [" << u.getId() << "] lv " << u.getLevel() << " xp " << u.getExperience()
                  << " hp " << u.getStats().getHealth() << "/" << u.getStats().getMaxHealth()
                  << " str " << u.getStats().getStrength() << " con " << u.getStats().getConstitution()
                  << " |";
        for (const std::string &s : u.getSkills())
            std::cout << " " << s;
        std::cout << " | injuries " << u.getInjuries().size() << std::endl;
    }
}

int main()
{
    const std::vector<Encounter> encounters = loadEncounters("resources/encounters.txt");
    const std::vector<Injury> injuries = loadInjuries("resources/injuries.txt");

    const ObjectiveType types[] = {ObjectiveType::Slay, ObjectiveType::Hold,
                                   ObjectiveType::Retrieve, ObjectiveType::Rescue};
    // Difficulty as a percentage of what the party rolls before luck (power + fit), from walkover
    // to hopeless and dense around parity -- luck spans +-1/6, so the interesting outcomes (a Hold
    // that half breaks, a Slay one blow short) live between 85 and 115. The party varies with the
    // seed (size, summoning tier, wounds, greed) for the same reason: every branch has to fire.
    const int percents[] = {60, 85, 92, 97, 100, 103, 108, 115, 140};
    const int SEEDS = 30;

    int cases = 0;
    for (ObjectiveType type : types)
        for (int percent : percents)
            for (int seed = 1; seed <= SEEDS; ++seed)
            {
                // A fresh world per case. The generator seed varies with the case so the party's
                // traits vary too: Retrieve's pushesOn and every traitFit depend on them.
                Roster roster;
                GameState state;
                Barracks barracks;
                std::mt19937 genRng(static_cast<unsigned int>(seed * 7919 + percent));
                Generator gen("resources", genRng);
                barracks.create("A");
                const int partySize = 1 + seed % 4;
                for (int i = 0; i < partySize; ++i)
                {
                    int id = state.nextUnitId++;
                    roster.addUnit(gen.generateUnit(id, state.necropolis, SUMMON_TIERS[seed % 3]));
                    barracks.assign(id, 0, roster);
                    Unit &u = roster.findUnitById(id);
                    if (seed % 4 == 1) // already hurt: wipes and deaths need a party that can break
                        u.takeDamage(u.getStats().getMaxHealth() * 3 / 4);
                    if (seed % 5 == 0) // greedy enough that Retrieve stays until it is caught
                    {
                        u.addSkill("Greedy");
                        u.addSkill("Curious");
                    }
                }

                // What the game's floorRoll adds up before luck: teamPower + teamFit.
                int expected = 0;
                for (int id : barracks.at(0).getMembersIds())
                {
                    const Unit &u = roster.findUnitById(id);
                    expected += u.getStats().getStrength() + u.getStats().getConstitution()
                                + traitFit(type, u.getSkills());
                }
                const int difficulty = std::max(10, expected * percent / 100);

                const int floors[] = {2, 6, 14};
                const int floor = floors[(seed / 3) % 3]; // awards, experience and incidents scale with it
                const int rounds = 3 + seed % 3;       // Hold and Rescue only; 0 otherwise, as makeObjective does
                const bool counted = type == ObjectiveType::Hold || type == ObjectiveType::Rescue;
                state.floorObjectives[floor] = Objective{type, difficulty, counted ? rounds : 0};

                Sortie sortie;
                sortie.partyIndex = 0;
                sortie.floor = floor;
                sortie.departedAt = 0;
                sortie.seed = static_cast<unsigned int>(seed * 104729 + difficulty * 31 + static_cast<int>(type));

                std::cout << "##### " << typeName(type) << " " << percent << "% d" << difficulty << " floor " << floor
                          << " seed " << sortie.seed << std::endl;
                bool cleared = resolveSortie(sortie, barracks.at(0), roster, state, encounters, injuries);
                std::cout << "--- cleared: " << (cleared ? "yes" : "no") << std::endl;
                printState(roster, state);
                ++cases;
            }

    std::cerr << cases << " cases" << std::endl;
    return 0;
}
