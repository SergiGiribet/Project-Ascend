#ifndef INJURY_H
#define INJURY_H

#include "Unit.h"
#include <string>
#include <vector>
#include <random>

struct Injury {
    std::string name;
    int strPenalty;
    int conPenalty;
};

std::vector<Injury> loadInjuries(const std::string &path);
// Pre: path points to a text file with one "name|strPenalty|conPenalty" entry per line.
// Post: Returns every entry in file order; throws std::runtime_error if the file is missing,
//       empty, or a non-empty line doesn't split into exactly three fields.

std::string applyInjury(Unit &unit, const std::vector<Injury> &bank, std::mt19937 &rng);
// Pre: bank must not be empty.
// Post: Picks a random Injury from bank and applies it permanently: STR and CON are reduced
//       by the injury's penalties (never below 1), and the injury's name is added to the
//       unit's skills. If the unit has "Reckless" or "Boaster", there is a 50% chance that
//       trait is replaced by "Alert" (no duplicate added if the unit already has it).

#endif