#ifndef FLOORSTATE_H
#define FLOORSTATE_H

#include <cstddef>
#include <string>
#include <variant>

// A floor in the middle of being resolved: what one step needs from the steps before it.
//
// Same rule as Sortie: only what cannot be worked out again. Anything that follows from the
// objective (danger, Slay's starting hp, Rescue's reach) is recomputed, never stored -- two
// copies drift. Which alternative `mission` holds always matches the floor's ObjectiveType;
// enterFloor sets it from the objective and nothing else may change it.

struct HoldState {
    int round = 1;              // the round about to be fought, 1-based as the chronicle prints it
    int failures = 0;           // rounds lost so far; more than half pushes them off the floor
};

struct SlayState {
    int enemyHp = 0;            // what it has left; enterFloor sets the starting value
    int failures = 0;           // counters taken so far; SLAY_COUNTERS_ENDURED ends the fight
};

struct RetrieveState {
    int passes = 0;             // searches made so far
    int exposure = 0;           // grows every pass; EXPOSURE_CAUGHT ends the search
    int extra = 0;              // essence found beyond the objective
    bool found = false;         // whether the objective itself has turned up
};

struct RescueState {
    int round = 1;              // as in Hold: the round about to be fought
    int progress = 0;           // ground covered towards the captive
};

struct FloorState {
    std::string cause;          // the encounter's death cause, copied: a save keeps what happened,
                                // not where in the data it came from
    int traitMod = 0;           // the trait event rolled on entry; it cannot be rolled again
    std::string lastLine;       // last chronicle line, so no two steps read the same
    std::variant<HoldState, SlayState, RetrieveState, RescueState> mission;
};

#endif
