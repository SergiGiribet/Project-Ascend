// Retrieve wipe test. NOT part of the game build (tasks.json compiles main.cpp and src/*.cpp
// only); it builds its own world in memory and never touches app/save.txt. From the repo root,
// with build/ (ignored by git) as the scratch directory:
//
//   mkdir -p build/retrieve_wipe && cp -r app/resources build/retrieve_wipe/
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -I app/src app/tests/retrieve_wipe.cpp app/src/*.cpp -o build/retrieve_wipe/retrieve_wipe
//   (cd build/retrieve_wipe && ./retrieve_wipe)
//
// Exit 0 and "PASS" is the pass.
//
// The rule: a party the floor takes before the floor is done has lost it, even on a Retrieve
// where the thing was already found -- nobody is left to carry it out. Up to b6e54fc such a wipe
// counted as taken and raised the tower record. The equivalence harness never reaches this case,
// so it needs its own test.
//
// The setup makes the case common: one Greedy unit, far too strong for the floor (the find comes
// on the first pass) and on 1 HP (any wound once the floor notices them kills). clearFloor always
// pays the floor before anything else, so "died and was not paid" means exactly "died before the
// floor was done". Pass: every such run is lost; and there is at least one, or this proves nothing.
#include "Incursion.h"

#include <iostream>
#include <sstream>

int main()
{
    const std::vector<Encounter> encounters = loadEncounters("resources/encounters.txt");
    const std::vector<Injury> injuries = loadInjuries("resources/injuries.txt");
    const int SEEDS = 500;

    int wipedUnpaid = 0;
    int takenAnyway = 0;
    for (unsigned seed = 0; seed < SEEDS; ++seed)
    {
        Roster roster;
        Unit searcher(1);
        searcher.setName("Searcher");
        searcher.setStats(Stats(1, 100, 500, 500));
        searcher.addSkill("Greedy");
        roster.addUnit(searcher);

        GameState state;
        state.essence = 0;
        std::vector<Postings::Moved> moved;
        Postings postings = Postings::fromSave(Barracks(), TrainingCamp(), state, moved);
        int party = postings.createParty("P");
        postings.assignToParty(1, party, roster);
        state.floorObjectives[1] = Objective{ObjectiveType::Retrieve, 60, 0};
        Sortie sortie;
        sortie.partyIndex = party;
        sortie.floor = 1;
        sortie.departedAt = 0;
        sortie.seed = seed;

        std::streambuf *old = std::cout.rdbuf();
        std::ostringstream sink;
        std::cout.rdbuf(sink.rdbuf());
        bool taken = resolveSortie(sortie, postings.party(party), roster, state, encounters, injuries);
        std::cout.rdbuf(old);

        bool died = !roster.contains(1);
        bool paid = state.essence > 0;
        if (died && !paid)
        {
            ++wipedUnpaid;
            if (taken)
            {
                if (takenAnyway == 0)
                    std::cout << "seed " << seed << ": wiped before the floor was done, and it counts as taken"
                              << std::endl;
                ++takenAnyway;
            }
        }
    }

    std::cout << wipedUnpaid << " wiped before the floor was done, " << takenAnyway
              << " of them counted as taken, in " << SEEDS << " seeds" << std::endl;
    if (wipedUnpaid == 0)
    {
        std::cout << "FAIL: no run was wiped before the floor was done, the case is not exercised" << std::endl;
        return 1;
    }
    if (takenAnyway > 0)
    {
        std::cout << "FAIL" << std::endl;
        return 1;
    }
    std::cout << "PASS" << std::endl;
    return 0;
}
