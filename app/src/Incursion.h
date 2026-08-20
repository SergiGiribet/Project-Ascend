#ifndef INCURSION_H
#define INCURSION_H

#include <random>
#include <string>
#include <vector>

#include "Roster.h"
#include "Team.h"
#include "GameState.h"
#include "Injury.h"

struct Encounter
{
    std::string description; // what awaits on the floor (no trailing period; the chronicle composes the sentence)
    std::string cause;       // matching death cause (lowercase, no trailing period; the tombstone composes it)
};

std::vector<Encounter> loadEncounters(const std::string &path);
// Pre: path points to a text file with one "description|cause" entry per line.
// Post: Returns every entry in file order; throws std::runtime_error if the file is missing,
//       empty, or a non-empty line contains no '|'.

void runScoutMission(int scoutId, Team &team, Roster &roster, GameState &state, std::mt19937 &rng);
// Pre: rng must be seeded.
// Post: Sends the unit with scoutId up ALONE to look at floor (state.highestFloor + 1), after
//       printing their own read of the odds and asking for confirmation; an id not on the roster
//       is refused and costs nothing, and so is declining. The mission comes from
//       state.floorObjectives, so it is the one that floor will really present. A survivor comes
//       back wounded (SCOUT_WOUND_MIN..MAX) and their claim is stored in state.floorReports for
//       that floor. If the risk roll or the wound kills them they bring nothing back -- the death
//       IS the report -- and they are recorded in state.necropolis, removed from the roster and
//       purged from team.

void runIncursion(Team &team, Roster &roster, GameState &state, const std::vector<Encounter> &encounters, const std::vector<Injury> &injuries, std::mt19937 &rng);
// Pre: Every id in team refers to a unit present in roster; encounters and injuries must not
//      be empty; rng must be seeded.
// Post: Runs one incursion floor by floor; the player chooses the start floor and whether to
//       keep climbing. Prints a notice and does nothing if team is empty. Increments
//       state.incursionCount.
//
//       ENTRY. Starting above floor 1 charges a toll of (start floor - 1) essence; an
//       unaffordable start floor falls back to floor 1, which is always free.
//
//       THE FLOOR. Each floor presents a random encounter and an objective taken from
//       state.floorObjectives, which keeps every floor's mission across incursions so that
//       scouted knowledge survives leaving the tower. Printed under the floor header: the
//       objective's briefing, then how well the team suits it (traitFit summed over the members
//       -- a signed modifier added to the team's power for this floor, shown as a plain-language
//       tier plus the number), then, if a scout claimed something else about this floor, a line
//       saying so and naming them (from state.floorReports). At most one trait event per floor
//       (Brave/Cowardly/Reckless) narrates a deed and modifies the roll.
//
//       RESOLUTION, HOLD. Hold objectives resolve round by round rather than on one roll. For
//       each of the objective's rounds the team rolls again -- power, fit and the trait modifier,
//       all recomputed every round, so a death mid-fight weakens the rest of it -- against the
//       floor's difficulty. A failed round wounds one member (HOLD_DMG_MIN..MAX) through the
//       wound path below. Giving ground on more than half the rounds means being pushed off the
//       floor; otherwise the floor is cleared, flawlessly or at a price, with no further wound:
//       the rounds already charged for it.
//
//       RESOLUTION, SLAY. Slay objectives are an exchange with whatever holds the floor. The
//       enemy starts with (difficulty * 3 / 2) health, never shown to the player as a number
//       (style guide 1.6). Each exchange the team rolls as above; a blow that beats the
//       difficulty takes (difficulty / 2 + the margin) off the enemy, so a stronger team lands
//       fewer and larger blows. SLAY_CRIT_CHANCE percent of landed blows also take half the
//       enemy's starting health -- the one fat tail that keeps a short fight from being a
//       calculation. A blow that misses draws a counterattack (SLAY_DMG_MIN..MAX through the
//       wound path below); after SLAY_COUNTERS_ENDURED of them the team pulls back off the floor,
//       and one line says how close they came, which is the only thing a loss gives them. The
//       narration of each exchange is chosen by the margin, so a near miss reads as a block and a
//       wide one as a dodge, without a second roll.
//
//       RESOLUTION, EVERY OTHER TYPE. One roll against the difficulty, in three tiers. Cleared
//       with ease: may still strike a residual incident (chance grows with floor depth and with
//       Boaster members, shrinks with Alert members, capped), whose own second roll decides
//       whether it is fatal or merely wounding, with its own flavor text -- an ironic one if the
//       victim is a Boaster. Cleared with difficulty: always wounds someone. Below that: the
//       tower overwhelms the team, one member falls and the incursion ends.
//
//       WOUNDS. The victim is chosen by a weighted lottery (Reckless/Boaster more likely, Alert
//       less likely, never forced). A wound the victim survives has a small chance of leaving a
//       permanent injury (applyInjury: a lasting STR/CON penalty, never healed by rest); a lethal
//       one runs the full death path.
//
//       CLIMBING. Before each climb prompt, prints a danger forecast for the next floor (4 tiers,
//       from the team's base power -- deliberately WITHOUT the trait fit, since the objective may
//       not be known -- against the next floor's difficulty). Scouting does not happen here: it
//       is its own errand outside the tower (runScoutMission), so the prompt offers only yes or
//       no.
//
//       AFTERWARDS. Each cleared floor yields its floor number in essence and XP to the
//       survivors, who may level up; state.highestFloor grows on a new record. Every casualty is
//       recorded in state.necropolis BEFORE being removed from roster and purged from team, with
//       the encounter's cause for difficulty, overwhelm and Hold-round deaths, and the incident's
//       own flavor text for residual incidents. Wounds are NOT healed here: whoever climbed
//       carries them home, and only units that stayed behind recover (Roster::healRested, called
//       by main). Permanent injuries and the dead both stay.

#endif