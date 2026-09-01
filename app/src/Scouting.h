#ifndef SCOUTING_H
#define SCOUTING_H

#include <random>
#include <string>

#include "Objective.h"
#include "Unit.h"

// What ONE unit brings back after slipping ahead to the next floor. Deliberately not a fact but
// a CLAIM: who goes decides what comes back, and an unreliable scout may omit or mistake things
// (2c-4). Everything the player then sees is derived from this claim, so a single wrong `type`
// is enough to make the whole forecast wrong on its own -- one lie, cascading.

struct Report {
    std::string scout;   // who went; every player-facing line is attributed to them by name
    int scoutId = -1;   // who made this report; -1 when nobody scouted this floor
    bool sawObjective = false;   // did they identify the mission at all
    Objective claimed{};   // what they say it is; only meaningful when sawObjective
    bool sawDanger = false;      // did they get close enough to judge the odds
    int bias = 0;            // added to the power they report: >0 tells it rosier than it is
};

Report scoutAhead(const Unit &scout, const Objective &objective, std::mt19937 &rng);
// Pre: rng must be seeded.
// Post: Returns what the scout claims about the objective. The mission is always identified and
//       always identified correctly for now (sawObjective and sawDanger true, claimed ==
//       objective); what varies is the DANGER they report. Every scout is off by up to
//       SCOUT_SPREAD either way, so who goes changes what comes back; Alert halves that. Boaster
//       and Cowardly each add SCOUT_SKEW in their own direction -- not more error but error with
//       a direction, which is worse, because the player cannot see which way it leans. Failing to
//       identify the mission at all (sawObjective false) arrives with 2e.

std::string describeReport(const Report &report);
// Pre: None.
// Post: Returns the player-facing line for what the scout brought back, attributed to them by
//       name and framed as their claim rather than as fact (style guide 1.5), with full
//       punctuation; notes it when they never got close enough to judge the odds. Reports what
//       the scout CLAIMS, never what is true.

std::string describeOdds(const std::string &scout, int risk);
// Pre: None.
// Post: Returns the scout's own read of their chances before setting off -- their judgement, not
//       the game's verdict (style guide 1.5) -- with full punctuation. Four tiers, grimmer as the
//       risk rises.

std::string describeMisreport(const Report &report);
// Pre: report.sawObjective must be true (there is no claim to contradict otherwise).
// Post: Returns the line printed when the floor turns out not to be what the scout claimed,
//       naming them so the mistake lands on the character and not on the game.

#endif
