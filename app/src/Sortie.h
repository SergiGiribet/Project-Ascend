#ifndef SORTIE_H
#define SORTIE_H

#include "FloorState.h"

#include <optional>
#include <random>

// A party that is inside the tower right now.
//
// What is stored here is only what cannot be worked out again: which party went, where, when they
// left and the seed -- and, once they are on the floor, where the floor stands and where the rng's
// sequence has got to. How long the sortie takes and when they are due are worked out from these
// whenever anyone asks, never stored.
//
// A sortie is played on the REAL units, not on copies: the party is physically in the tower, and
// every step it plays hurts the people in it, there and then. What must hold is that nothing at
// home reaches in -- Roster::healRested is handed awayUnitIds for exactly that.
//
// The save is still no spoiler. A sortie in progress stores how far the floor has got, never how it
// ends: the rolls still to come have not been made, and the outcome does not exist anywhere until
// it is played.
struct Sortie {
    int partyIndex = -1;            // Index into Barracks; a party has at most one sortie at a time.
                                    // Kept valid across disbands by disbandParty, which is the only
                                    // thing allowed to make indices shift.
    int floor = 0;                  // Which floor they went to.
    long long departedAt = 0;       // nowSeconds() at the moment they left. Real time, not game time.
    unsigned int seed = 0;          // Seeds rng when the party enters the floor, and only then, so the
                                    // sortie is reproducible and does NOT depend on what the player did
                                    // at home while they were away.
    std::optional<FloorState> fs;   // nullopt until the party enters the floor; from then on, where
                                    // the floor stands between one step and the next.
    std::mt19937 rng;               // Means nothing until fs has a value: entering seeds it from seed,
                                    // and every step draws from it, so mid-floor it carries the
                                    // sequence on from where it stopped.
};

#endif