#ifndef INCURSION_H
#define INCURSION_H

#include <random>
#include <string>
#include <vector>
#include <optional>

#include "Roster.h"
#include "Team.h"
#include "GameState.h"
#include "Injury.h"
#include "Barracks.h"
#include "TrainingCamp.h"
#include "Sortie.h"

struct Encounter
{
    std::string description; // what awaits on the floor (no trailing period; the chronicle composes the sentence)
    std::string cause;       // matching death cause (lowercase, no trailing period; the tombstone composes it)
};

std::vector<Encounter> loadEncounters(const std::string &path);
// Pre: path points to a text file with one "description|cause" entry per line.
// Post: Returns every entry in file order; throws std::runtime_error if the file is missing,
//       empty, or a non-empty line contains no '|'.

void runScoutMission(int scoutId, Barracks &barracks, Roster &roster, GameState &state, std::mt19937 &rng);
// Pre: rng must be seeded.
// Post: Sends the unit with scoutId up ALONE to look at floor (state.highestFloor + 1), after
//       printing their own read of the odds and asking for confirmation; an id not on the roster
//       is refused and costs nothing, and so is declining. The mission comes from
//       state.floorObjectives, so it is the one that floor will really present. A survivor comes
//       back wounded (SCOUT_WOUND_MIN..MAX) and their claim is stored in state.floorReports for
//       that floor. If the risk roll or the wound kills them they bring nothing back -- the death
//       IS the report -- and they are recorded in state.necropolis, removed from the roster and
//       purged from every party (Barracks::purgeDead): a scout is picked from the whole roster,
//       so they may belong to any party, or to none.

std::optional<Sortie> launchSortie(int partyIndex, const Team &team, const Roster &roster,
                                   GameState &state, std::mt19937 &rng);
// Pre: partyIndex must name a party that exists in the Barracks, and `team` must be that party.
// Post: Runs the departure dialogue -- refuses an empty party, asks which floor, shows what is
//       known about it (a forecast if it has been stood on, a scout's account if one was bought,
//       nothing at all otherwise) and asks for confirmation. Returns the Sortie they left on, or
//       std::nullopt if they did not go -- in which case NOTHING has been spent or changed.
//       `team` and `roster` are const on purpose: deciding to enter cannot hurt anyone, and the
//       compiler is what says so.
//       If the floor is new its objective is rolled here and cached in state.floorObjectives, so
//       what the player was shown is what they will face. The seed is drawn from `rng` here and
//       never again: from this line on, the sortie owes nothing to anything that happens at home.

bool resolveSortie(const Sortie &sortie, Team &team, Roster &roster, GameState &state,
                   const std::vector<Encounter> &encounters, const std::vector<Injury> &injuries);
// Pre: `team` must be the party named by sortie.partyIndex, and the floor's objective must already
//      be in state.floorObjectives -- launchSortie is what puts it there.
// Post: Plays the floor out and writes down what it cost: essence and experience awarded, the dead
//       struck from the roster and entered in the Necropolis, the survivors' wounds kept, and
//       state.highestFloor raised if the floor was new. Returns whether the floor was taken.
//       Takes no rng and must not: it builds its own from sortie.seed. That is what makes looking
//       twice give the same answer, and what stops anything the player did at home while the party
//       was away from reaching into the tower.

void catchUp(Barracks &barracks, Roster &roster, GameState &state, TrainingCamp &camp,
             const std::vector<Encounter> &encounters, const std::vector<Injury> &injuries,
             std::mt19937 &rng);
// Pre: Every sortie in state.sorties must name a party that still exists.
// Post: Brings the world up to the present. Any sortie whose time is up is taken out of
//       state.sorties and played out -- printing everything that happened up there, in one go,
//       because nobody was watching. state.lastSeen is advanced to now.
//       A system clock that has been wound BACK does not move time at all: `now` is clamped to
//       lastSeen, so a party cannot be un-returned by changing the date. Winding it forward while
//       the game is closed is not preventable and is accepted.
//       Called on every main-menu redraw, so it must be cheap and must be safe to call when
//       nothing is due -- which is nearly always.

void printSorties(const GameState &state, const Barracks &barracks);
// Pre: Every sortie in state.sorties must name a party that still exists.
// Post: Prints one line per party currently inside the tower -- which party, which floor, and how
//       long until they are due -- or nothing at all when none are out. This is the only place the
//       player is told that a sortie exists, so it runs before the menu, every time.

#endif