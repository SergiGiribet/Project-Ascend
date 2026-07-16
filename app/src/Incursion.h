#ifndef INCURSION_H
#define INCURSION_H

#include <random>

#include "Roster.h"
#include "Team.h"
#include "GameState.h"

void runIncursion(Team &team, Roster &roster, GameState &state, std::mt19937 &rng);
// Pre:
// Post: Resolves an incursion floor by floor: survivors gain XP, casualties are removed from the roster and purged from the team.

#endif