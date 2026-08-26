// Barracks is the set of parties the player has organized. It owns the Team objects; nothing
// else does.
//
// IMPORTANT: it exists for ONE reason -- to defend an invariant no single Team can defend by
// itself: a unit belongs to AT MOST ONE party. That rule is about the space between parties, so
// it needs an owner above them. Every path that puts a unit in a party goes through assign(),
// which frees the unit from wherever it was first; if that logic lived in the menus instead,
// it would be correct until the day a third menu forgot to call it.
//
// The payoff shows up immediately: because a dead unit can only ever be in one party, purging
// the party that climbed is enough to keep every party consistent with the roster.
//
// Invariant: no unit id appears in two parties; every id in every party refers to a living unit
// present in the roster.

#ifndef BARRACKS_H
#define BARRACKS_H

#include "Team.h"

#include <map>
#include <string>
#include <vector>

class Barracks
{
public:
    // Consultors -------------------------------------------------------------------------------
    int count() const;
    // Pre: None
    // Post: Returns how many parties exist (0 if none).

    Team &at(int index);
    
    const Team &at(int index) const;
    // Pre: 0 <= index < count() (throws std::out_of_range otherwise).
    // Post: Returns the party at that index. WARNING: the reference is only valid until the next
    //       create() or disband(); both may move the parties in memory, so never hold it across
    //       one (the same trap as a Unit & held across a roster removal).

    int teamOfUnit(int unitId) const;
    // Pre: None
    // Post: Returns the index of the party the unit belongs to, or -1 if it belongs to none.

    std::map<int, std::string> memberTags() const;
    // Pre: None
    // Post: Returns, for every unit that belongs to a party, its id mapped to that party's name.
    //       This is what the roster screen prints as a marker, so a unit shows WHICH party it is
    //       in rather than merely that it is in one. Units in no party are absent from the map.

    // Modifiers --------------------------------------------------------------------------------
    int create(const std::string &name);
    // Pre: None
    // Post: Creates an empty party with that name and returns its index. Does not enforce a
    //       minimum or a maximum number of parties: keeping at least one for the tower to send
    //       is the caller's business.

    void disband(int index);
    // Pre: 0 <= index < count() (throws std::out_of_range otherwise).
    // Post: Removes that party; its members belong to no party afterwards and are otherwise
    //       untouched. Every later index shifts down by one.

    void assign(int unitId, int index, const Roster &roster);
    // Pre: 0 <= index < count(), unitId must be present in roster, and the target party must not
    //      be full (throws std::runtime_error / std::out_of_range otherwise).
    // Post: Puts the unit in that party, FIRST releasing it from any party it already belonged
    //       to -- which is what makes the class invariant impossible to break from outside.
    //       Assigning a unit to the party it is already in leaves everything as it was.

    void release(int unitId);
    // Pre: None
    // Post: Removes the unit from whatever party it belonged to; does nothing if it belonged to
    //       none.

    void purgeDead(const Roster &roster);
    // Pre: None
    // Post: Removes from every party any member whose id is no longer present in the roster.
    //       Empty parties are kept: a party that lost everyone is information, not garbage.

    // Display ----------------------------------------------------------------------------------
    void printAll(const Roster &roster) const;
    // Pre: Every id in every party must be present in roster (the class invariant).
    // Post: Prints each party in index order, numbered as the menus number them, or a notice if
    //       no party exists.

private:
    std::vector<Team> teams_;
};

#endif // BARRACKS_H
