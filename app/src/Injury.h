#ifndef INJURY_H
#define INJURY_H

#include <string>
#include <vector>
#include <random>

class Unit; // forward declaration: the free functions below only take a Unit&, so the
            // declarations don't need Unit's full definition. This breaks the include
            // cycle (Unit.h now includes Injury.h to use struct Injury as a member).

struct Injury
{
    std::string name;
    int strPenalty;
    int conPenalty;
};

std::vector<Injury> loadInjuries(const std::string &path);
// Pre: path points to a text file with one "name|strPenalty|conPenalty" entry per line.
// Post: Returns every entry in file order; throws std::runtime_error if the file is missing,
//       empty, or a non-empty line doesn't split into exactly three fields.

std::string injuryLabel(const Injury &injury);
// Pre: None
// Post: Returns the injury's name followed by its stat cost in parentheses, e.g.
//       "One-Handed (-3 STR)" or "Broken Ribs (-2 STR, -3 CON)". Only non-zero penalties
//       are shown, and the parenthetical is omitted entirely if both penalties are zero.

std::string applyInjury(Unit &unit, const std::vector<Injury> &bank, std::mt19937 &rng);
// Pre: bank must not be empty.
// Post: Picks a random Injury from bank and applies it permanently: STR and CON are reduced
//       by the injury's penalties (never below 1), and the injury's name is added to the
//       unit's injuries (kept apart from traits). If the unit has "Reckless" or "Boaster",
//       there is a 50% chance that trait is replaced by "Alert" (no duplicate added if the
//       unit already has it). Returns the applied injury's name.

#endif