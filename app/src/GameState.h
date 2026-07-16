// GameState (GDD §3.4) represents the current state of the game, including the roster of living units, the game map, and other relevant information.

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Necropolis.h"

struct GameState {
    int highestFloor = 0;
    // Pre: None
    // Post: Highest tower floor ever conquered across all incursions this session

    int incursionCount = 0;
    Necropolis necropolis;
};

#endif 