// GameState (GDD §3.4) bundles the progress that persists between incursions: the tower record,
// how many incursions have been launched, the essence economy, and the Necropolis (the memory
// of the fallen). The roster and the team live in main and are passed around separately.

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Necropolis.h"

struct GameState {
    int highestFloor = 0;   // Highest tower floor ever cleared this session; never decreases.
    int incursionCount = 0; // Total incursions launched this session.
    Necropolis necropolis;  // Registry of the fallen; feeds hook generation.
    int essence = 25;       // The tower's currency. Earned by clearing floors (+floor number);
                            // spent on summoning (invokeCost) and as an entry toll of
                            // (start floor - 1) when entering an incursion above floor 1.
    int invokeCost = 5;     // Essence price of summoning a new unit.
};

#endif
