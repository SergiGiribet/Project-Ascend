// GameState (GDD §3.4) bundles the progress that persists between incursions: the tower record,
// how many incursions have been launched, and the Necropolis (the memory of the fallen).
// The roster and the team live in main and are passed around separately.

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Necropolis.h"

struct GameState {
    int highestFloor = 0;   // Highest tower floor ever cleared this session; never decreases.
    int incursionCount = 0; // Total incursions launched this session.
    Necropolis necropolis;  // Registry of the fallen; feeds hook generation.
};

#endif 