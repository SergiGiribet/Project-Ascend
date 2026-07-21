#ifndef INCURSION_H
#define INCURSION_H

#include <random>
#include <string>
#include <vector>

#include "Roster.h"
#include "Team.h"
#include "GameState.h"

struct Encounter
{
    std::string description; // what awaits on the floor (no trailing period; the chronicle composes the sentence)
    std::string cause;       // matching death cause (lowercase, no trailing period; the tombstone composes it)
};

std::vector<Encounter> loadEncounters(const std::string &path);
// Pre: path points to a text file with one "description|cause" entry per line.
// Post: Returns every entry in file order; throws std::runtime_error if the file is missing,
//       empty, or a non-empty line contains no '|'.

void runIncursion(Team &team, Roster &roster, GameState &state, const std::vector<Encounter> &encounters, std::mt19937 &rng);
// Pre: Every id in team refers to a unit present in roster; encounters must not be empty;
//      rng must be seeded.
// Post: Runs one incursion floor by floor (the player chooses the start floor and whether to
//       keep climbing). Entering above floor 1 charges a toll of (start floor - 1) essence;
//       an unaffordable start floor falls back to floor 1, which is always free. Each
//       cleared floor yields its floor number in essence. Each floor presents a random
//       encounter; at most one trait event per floor (Brave/Cowardly/Reckless) may modify the
//       team's roll. A floor cleared with ease can still strike a residual incident (chance
//       grows with floor depth and with Boaster members, shrinks with Alert members, capped);
//       when it does, a second roll decides if it is fatal or just a wound, with its own
//       flavor text (an ironic one if the victim is a Boaster). A floor cleared with
//       difficulty always wounds someone. Either way the victim is chosen by a weighted
//       lottery (Reckless/Boaster more likely, Alert less likely, never forced). Before each
//       climb prompt, prints a danger forecast for the next floor (4 tiers, computed from
//       current team power vs the next floor's danger). Increments state.incursionCount;
//       survivors gain XP and may level up; each casualty is recorded in state.necropolis
//       BEFORE being removed from roster and purged from team, with the encounter's cause for
//       difficulty/overwhelm deaths or the incident's own flavor text for residual incidents;
//       state.highestFloor grows if a new record floor is cleared. Survivors are fully healed
//       when the incursion ends (the dead stay dead). Prints a notice and does nothing if team
//       is empty.

#endif