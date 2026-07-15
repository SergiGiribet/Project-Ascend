// Team is the set of units chosen to enter the next incursion.
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

#ifndef TEAM_H
#define TEAM_H

#include "Roster.h"
#include <vector>

class Team {
public:
    // Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Team();

    // Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void addMember(int id, const Roster &roster);
    // Pre: id must be a valid unit id present in the roster and not already in
    //      the team (throws std::runtime_error otherwise).
    // Post: Adds the unit with the specified id to the team.

    void removeMember(int id);
    // Pre: None
    // Post: Removes the unit with the specified id from the team; does nothing if no
    //      unit has that id.

    void purgeDeadMembers(const Roster &roster);
    // Pre: roster must be a valid Roster object.
    // Post: Removes from the team any members whose ids are no longer present in the roster

    // Display ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void printTeam(const Roster &roster) const;
    // Pre: roster must be a valid Roster object.
    // Post: Prints the composition of the team, resolving ids to names/stats through the
    //       provided roster.

private:
    std::vector<int> memberIds_;
};

#endif // TEAM_H