// Camp <-> party regression test. NOT part of the game build (tasks.json compiles main.cpp and
// src/*.cpp only); it builds its own world in memory and never touches app/save.txt. From the repo
// root, with build/ (ignored by git) as the scratch directory:
//
//   mkdir -p build/camp_party && cp -r app/resources build/camp_party/
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -I app/src app/tests/camp_party.cpp app/src/*.cpp -o build/camp_party/camp_party
//   (cd build/camp_party && ./camp_party)
//
// Exit 0 and "PASS" is the pass.
//
// The bug: a unit could be a camp trainee and a party member at once (main.cpp assigned to a party
// without asking the camp, and a save could hold both), and nobody told the camp when that unit
// died. catchUp resolved the sortie, then camp.tick looked the dead unit up and threw -- before the
// game saved, so the same sortie replayed and threw again on every launch. A scout taken from the
// camp who died left the same dead id behind.
//
// On the code before Postings (04da8fb) the first scenario failed 200 of 200 seeds. The code now
// goes through Postings, which that commit does not have, so the check against the old behaviour is
// done by mutation instead: undo each part of the fix and the scenario that guards it must fail.
//
// Four scenarios, each must hold:
//   1. menu path   -- a trainee assigned to a party leaves the camp; catchUp never throws and a
//                     unit that died is in neither place (and dies on some seed, or proves nothing).
//   2. save, home  -- a save with the unit in both places, its party at home: it stays in the camp.
//   3. save, tower -- the same with its party in the tower: it stays in the party, leaves the camp,
//                     its trainees are freed; catchUp never throws.
//   4. scout       -- a trainee sent to scout who dies is gone from the camp; ticking it is safe.
#include "Incursion.h"

#include <algorithm>
#include <iostream>
#include <sstream>

static int failures = 0;

static void check(bool ok, const std::string &what)
{
    if (!ok)
    {
        std::cout << "  FAIL: " << what << std::endl;
        ++failures;
    }
}

// Two units: 1 trains, 2 is the one who gets moved and, with 1 HP, the one who dies.
static Roster makeRoster()
{
    Roster roster;
    Unit trainer(1);
    trainer.setName("Trainer");
    roster.addUnit(trainer);
    Unit victim(2);
    victim.setName("Victim");
    victim.setStats(Stats(1, 100, 1, 1));
    roster.addUnit(victim);
    return roster;
}

// Runs fn with std::cout swallowed, and std::cin reading `input`. Returns the exception text, or ""
// if nothing was thrown.
template <typename F>
static std::string quietly(const std::string &input, F fn)
{
    std::streambuf *oldOut = std::cout.rdbuf();
    std::streambuf *oldIn = std::cin.rdbuf();
    std::ostringstream sink;
    std::istringstream source(input);
    std::cout.rdbuf(sink.rdbuf());
    std::cin.rdbuf(source.rdbuf());
    std::string error;
    try
    {
        fn();
    }
    catch (const std::exception &e)
    {
        error = e.what();
    }
    std::cout.rdbuf(oldOut);
    std::cin.rdbuf(oldIn);
    return error;
}

int main()
{
    const std::vector<Encounter> encounters = loadEncounters("resources/encounters.txt");
    const std::vector<Injury> injuries = loadInjuries("resources/injuries.txt");
    const int SEEDS = 200;

    std::cout << "1. menu path" << std::endl;
    {
        int deaths = 0;
        int throws = 0;
        int posted = 0;
        for (unsigned seed = 0; seed < SEEDS; ++seed)
        {
            Roster roster = makeRoster();
            GameState state;
            std::vector<Postings::Moved> moved;
            Postings postings = Postings::fromSave(Barracks(), TrainingCamp(), state, moved);
            postings.addTrainer(1, roster);
            postings.assignTrainee(1, 2, roster);
            int party = postings.createParty("P");
            postings.assignToParty(2, party, roster); // the path main.cpp takes
            if (seed == 0)
                check(!postings.camp().contains(2), "a trainee assigned to a party is still in the camp");

            state.floorObjectives[1] = Objective{ObjectiveType::Hold, 500, 3};
            Sortie sortie;
            sortie.partyIndex = party;
            sortie.floor = 1;
            sortie.departedAt = 0;
            sortie.seed = seed;
            state.sorties.push_back(sortie);
            std::mt19937 rng(42);
            std::string error = quietly("", [&] {
                catchUp(postings, roster, state, encounters, injuries, rng);
            });
            if (!error.empty())
            {
                if (throws == 0)
                    std::cout << "  seed " << seed << ": catchUp throws: " << error << std::endl;
                ++throws;
            }
            if (!roster.contains(2))
            {
                ++deaths;
                if (postings.camp().contains(2) || postings.barracks().teamOfUnit(2) != -1)
                    ++posted;
            }
        }
        check(throws == 0, std::to_string(throws) + " of " + std::to_string(SEEDS) + " seeds throw");
        check(posted == 0, "in " + std::to_string(posted) + " seeds the dead unit is still posted somewhere");
        check(deaths > 0, "the victim never died, the scenario is not exercised");
        std::cout << "  " << deaths << " deaths, " << throws << " throws in " << SEEDS << " seeds" << std::endl;
    }

    std::cout << "2. save, party at home" << std::endl;
    {
        Roster roster = makeRoster();
        Barracks barracks;
        TrainingCamp camp;
        camp.addTrainer(1, roster);
        camp.assignTrainee(1, 2, roster);
        int party = barracks.create("P");
        barracks.assign(2, party, roster); // both places, as an old save can hold it
        GameState state;
        std::vector<Postings::Moved> moved;
        Postings postings = Postings::fromSave(barracks, camp, state, moved);
        check(postings.camp().contains(2), "the unit left the camp");
        check(postings.barracks().teamOfUnit(2) == -1, "the unit is still in the party");
        check(moved.size() == 1 && moved[0].unitId == 2 && moved[0].kept == Postings::Moved::Kept::Camp,
              "Moved should say: unit 2, kept in the camp");
    }

    std::cout << "3. save, party in the tower" << std::endl;
    {
        int throws = 0;
        for (unsigned seed = 0; seed < SEEDS; ++seed)
        {
            Roster roster = makeRoster();
            Barracks barracks;
            TrainingCamp camp;
            camp.addTrainer(1, roster);
            camp.assignTrainee(1, 2, roster);
            int party = barracks.create("P");
            barracks.assign(1, party, roster); // the TRAINER is out, with 2 under them at home
            barracks.assign(2, party, roster); // and 2 is out as well
            GameState state;
            state.floorObjectives[1] = Objective{ObjectiveType::Hold, 500, 3};
            Sortie sortie;
            sortie.partyIndex = party;
            sortie.floor = 1;
            sortie.departedAt = 0;
            sortie.seed = seed;
            state.sorties.push_back(sortie);
            std::vector<Postings::Moved> moved;
            Postings postings = Postings::fromSave(barracks, camp, state, moved);
            if (seed == 0)
            {
                check(!postings.camp().contains(1) && !postings.camp().contains(2),
                      "a unit whose party is in the tower is still in the camp");
                check(postings.barracks().teamOfUnit(1) == party && postings.barracks().teamOfUnit(2) == party,
                      "a unit whose party is in the tower left it");
                bool freed = std::any_of(moved.begin(), moved.end(), [](const Postings::Moved &m) {
                    return m.unitId == 1 && m.kept == Postings::Moved::Kept::Party
                        && std::find(m.freedTrainees.begin(), m.freedTrainees.end(), 2) != m.freedTrainees.end();
                });
                check(freed, "Moved should say: trainer 1 kept in the party, trainee 2 freed");
            }
            std::mt19937 rng(42);
            std::string error = quietly("", [&] {
                catchUp(postings, roster, state, encounters, injuries, rng);
            });
            if (!error.empty())
                ++throws;
        }
        check(throws == 0, std::to_string(throws) + " of " + std::to_string(SEEDS) + " seeds throw");
    }

    std::cout << "4. scout from the camp" << std::endl;
    {
        Roster roster = makeRoster();
        GameState state;
        std::vector<Postings::Moved> moved;
        Postings postings = Postings::fromSave(Barracks(), TrainingCamp(), state, moved);
        postings.addTrainer(1, roster);
        postings.assignTrainee(1, 2, roster);
        std::mt19937 rng(42);
        std::string error = quietly("1\n", [&] { // "1" answers "Send them up?"
            runScoutMission(2, postings, roster, state, rng);
        });
        check(error.empty(), "the scouting run throws: " + error);
        check(!roster.contains(2), "the 1 HP scout survived, the scenario is not exercised");
        check(!postings.camp().contains(2), "the dead scout is still in the camp");
        error = quietly("", [&] { postings.tickCamp(roster, injuries, rng); });
        check(error.empty(), "ticking the camp after the scout died throws: " + error);
    }

    if (failures > 0)
    {
        std::cout << "FAIL (" << failures << ")" << std::endl;
        return 1;
    }
    std::cout << "PASS" << std::endl;
    return 0;
}
