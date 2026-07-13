// Roster (GDD §3.3) is the collection of all LIVING units in the game.
// It wraps a std::vector<Unit> and is the single owner of every Unit object: any other class that needs a unit
// refers to it by id and asks this class to resolve it.
//
// Responsibilities:
//   - Add a newly invoked unit (takes ownership).
//   - Find a unit by id (lookup used by Team and the incursion logic).
//   - Remove a unit by id (permadeath: called AFTER the Necropolis has recorded the death).
//   - Print the full roster (for the "View stats" menu).
//
// Invariant: every unit stored here is alive; ids are unique within the collection.

#ifndef ROSTER_H
#define ROSTER_H

#include "Unit.h"
#include <vector>
#include <iostream>

class Roster {
public:
    Roster();

    void addUnit(const Unit &unit);
    // Pre: unit must be a valid Unit object with a unique id.
    // Post: Adds the unit to the roster, taking ownership of it.
    
    Unit &findUnitById(int id);
    // Pre: id must be a valid unit id (throws std::runtime_error otherwise).
    // Post: Returns a reference to the unit with the specified id; changes made through it affect the roster.

    const Unit &findUnitById(int id) const;
    // Pre: id must be a valid unit id (throws std::runtime_error otherwise).
    // Post: Returns a read-only reference to the unit with the specified id.

    void removeUnitById(int id);
    // Pre: None
    // Post: Removes the unit with the specified id from the roster; does nothing if no unit has that id.

    void printRoster() const;
    // Pre: None
    // Post: Prints the full roster.

private:
    std::vector<Unit> units_;
    // Pre: None
    // Post: Stores the collection of living units in the game.
    // Invariant: every unit in this vector is alive; ids are unique within the collection.
};

#endif // ROSTER_H