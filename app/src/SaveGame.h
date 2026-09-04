#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "Barracks.h"
#include "TrainingCamp.h"
#include "Roster.h"
#include "GameState.h"

#include <string>

void saveGame(const std::string &path, const GameState &state, const Roster &roster,
              const Barracks &barracks, const TrainingCamp &camp);
// Pre: None.
// Post: Writes the whole game to path, overwriting whatever was there. One slot, always: the
//       save exists so the player can close the game, not so they can undo a death.
//       Throws std::runtime_error if the file cannot be opened.

bool loadGame(const std::string &path, GameState &state, Roster &roster,
              Barracks &barracks, TrainingCamp &camp);
// Pre: state, roster, barracks and camp should be empty; loading into a game in progress is not
//      supported.
// Post: Fills all four from the file and returns true. Returns FALSE and touches nothing if
//       the file does not exist -- that is not an error, it is a first run. Parties are
//       written after the units they hold, and so is the training camp, so that every member
//       already exists on the roster by the time something asks for it.


#endif