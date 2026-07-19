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
    // Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Roster();
    // Pre: None
    // Post: Creates an empty roster.

    // Consultors --------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Unit &findUnitById(int id);
    // Pre: id must be a valid unit id (throws std::runtime_error otherwise).
    // Post: Returns a reference to the unit with the specified id; changes made through it affect
    //       the roster. The reference is invalidated by addUnit/removeUnitById: use it
    //       immediately, never store it.

    const Unit &findUnitById(int id) const;
    // Pre: id must be a valid unit id (throws std::runtime_error otherwise).
    // Post: Returns a read-only reference to the unit with the specified id. Same lifetime rule
    //       as the non-const overload: invalidated by addUnit/removeUnitById.

    bool contains(int id) const;
    // Pre: None
    // Post: Returns true if a unit with the specified id is in the roster; false otherwise.

    // Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void addUnit(const Unit &unit);
    // Pre: unit must be a valid Unit object with a unique id.
    // Post: Adds the unit to the roster, taking ownership of it.

    void removeUnitById(int id);
    // Pre: None
    // Post: Removes the unit with the specified id from the roster; does nothing if no unit has that id.

    // Display --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void printRoster(const std::vector<int> &teamIds) const;
    // Pre: None
    // Post: Prints one line per unit, or a notice if the roster is empty. Units whose id
    //       appears in teamIds are highlighted and tagged "[in team]".

private:
    std::vector<Unit> units_;
    // Invariant: every unit in this vector is alive; ids are unique within the collection.
};

#endif // ROSTER_H