#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include <string>
#include <random>
#include <vector>

// A floor's MISSION; what the team must accomplish to clear the floor. It replaces Phase 1's
// single scalar danger. The type decides which team attributes will matter; difficulty is the
// magnitude that used to be `danger`. Neither is derivable from the floor number: the type is
// random and the difficulty swings around its floor, which is what makes a floor worth scouting
// instead of merely counting.

enum class ObjectiveType {
    Slay,               // defeat what holds the floor
    Hold,               // survive a number of rounds
    Retrieve,           // find and carry off something hidden on the floor
    Rescue,             // free a trapped unit (it joins the roster on success)
};

struct Objective {
    ObjectiveType type;
    int difficulty; // floor-scaled magnitude (the old `danger`); every type uses it
    int rounds;     // a count of rounds, and it means a different thing to each type that uses
                    // it: for Hold, how long they must last; for Rescue, how long the captive
                    // has. 0 for Slay and Retrieve, whose length is not fixed in advance
};

Objective makeObjective(int floor, std::mt19937 &rng);
// Pre: floor >= 1.
// Post: Returns a random Objective for the floor: type chosen uniformly, and any type-specific
//       field set (rounds for Hold, 0 otherwise). Difficulty is the floor's own: it scales with
//       the floor AND carries a swing of up to FLOOR_SWING floors either way, so a floor may sit
//       harder or softer than its number says. Caller is expected to roll this ONCE per floor and
//       remember it (Incursion's objectiveFor caches into state.floorObjectives) -- the swing is
//       the floor's identity, not a per-visit surprise, and it is what a scout is sent to learn.

std::string describeObjective(const Objective &objective);
// Pre: None.
// Post: Returns the player-facing briefing line for the objective (full punctuation included),
//       e.g. "Cut down what holds the floor." or "Hold the line for 3 rounds."

int traitFit(ObjectiveType type, const std::vector<std::string> &traits);
// Pre: None (an empty trait list is valid).
// Post: Returns how well the trait list suits the objective type: the sum of the per-type
//       weights of every listed trait (+-8 strong fit, +-4 mild, 0 irrelevant). Traits with no
//       entry in the table are ignored; a list with none of them scores 0. Throws
//       std::out_of_range if type has no column, i.e. a new ObjectiveType was added without
//       extending the table.

std::string describeFit(int fit);
// Pre: None.
// Post: Returns the player-facing line for a traitFit total: a plain-language tier plus the
//       signed modifier, with full punctuation, e.g. "This team is well suited to it (+18)."


#endif