#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "Barracks.h"
#include "Roster.h"
#include "GameState.h"

#include <string>

void saveGame(const std::string &path, const GameState &state, const Roster &roster,
              const Barracks &barracks);
// Pre: None.
// Post: Writes the whole game to path, overwriting whatever was there. One slot, always: the
//       save exists so the player can close the game, not so they can undo a death.
//       Throws std::runtime_error if the file cannot be opened.

bool loadGame(const std::string &path, GameState &state, Roster &roster,
              Barracks &barracks);
// Pre: state, roster and barracks should be empty; loading into a game in progress is not
//      supported.
// Post: Fills all three from the file and returns true. Returns FALSE and touches nothing if
//       the file does not exist -- that is not an error, it is a first run. Parties are
//       written after the units they hold and read back in the same order, so every member
//       already exists on the roster by the time a party asks for it.


#endif