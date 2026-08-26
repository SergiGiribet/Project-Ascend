// Team is one party: the set of units chosen to enter a floor together. The player may keep
// several, so a Team carries a name; Barracks owns them all.
// IMPORTANT: it does NOT store Unit objects. Storing Units would store COPIES, and damage/XP dealt during
// an incursion would be applied to the copies while the roster stayed untouched. Instead, Team stores the
// ids of its members (std::vector<int>) and resolves them through the Roster when needed.
//
// Responsibilities:
//   - Add a member by id (validating it exists in the roster and is not already in the team).
//   - Remove a member by id.
//   - Print the team composition (needs the roster passed as const reference to resolve ids to names/stats).
//   - Purge ids of units that died in the incursion (kept consistent with the roster after each incursion).
//
// Invariant: every id stored here refers to a living unit present in the roster; no duplicates.
// A second rule -- that no unit belongs to two parties -- cannot be checked from here, because it
// is about the space between parties. Barracks owns it.

#ifndef TEAM_H
#define TEAM_H

#include "Roster.h"
#include <vector>
#include <string>

class Team
{
public:
    // Constants ---------------------------------------------------------------------------------------------------------------------------------------------------------------------
    static const int MAX_MEMBERS = 5;

    // Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Team(const std::string &name);
    // Pre: None
    // Post: Creates an empty party under that name. There is deliberately no default constructor:
    //       a party with no name is of no use to anyone once Barracks exists, and leaving one
    //       available would only leave a way to build a broken one.

    // Consultors -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
    const std::vector<int> &getMembersIds() const;
    // Pre: None
    // Post: Returns the ids of the team members.

    const std::string &getName() const;
    // Pre: None
    // Post: Returns the name of the team.

    // Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void addMember(int id, const Roster &roster);
    // Pre: id must be a valid unit id present in the roster, not already in the team,
    //      and the team must not be full (MAX_MEMBERS) (throws std::runtime_error otherwise).
    // Post: Adds the unit with the specified id to the team.

    void removeMember(int id);
    // Pre: None
    // Post: Removes the unit with the specified id from the team; does nothing if no
    //      unit has that id.

    void purgeDeadMembers(const Roster &roster);
    // Pre: None
    // Post: Removes from the team any members whose ids are no longer present in the roster.

    // Display ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void printTeam(const Roster &roster) const;
    // Pre: Every id in the team must be present in roster (the class invariant).
    // Post: Prints a header with the party name and its size, then one line per member
    //       (resolved through the roster) plus its hook as an indented sub-line. An empty party
    //       still prints its name: with several parties, WHICH one is empty is the information.

private:
    std::vector<int> memberIds_;
    std::string name_;
};

#endif // TEAM_H