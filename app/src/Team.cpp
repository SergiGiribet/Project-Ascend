#include "Team.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

// Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
Team::Team() : memberIds_() {}

// Consultors --------------------------------------------------------------------------------------------------------------------------------------------------------------------
const std::vector<int> &Team::getMembersIds() const
{
    return memberIds_;
}

// Modifiers ---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Team::addMember(int id, const Roster &roster)
{
    if (!roster.contains(id))
    {
        throw std::runtime_error("Unit with id " + std::to_string(id) + " is not in the roster.");
    }

    if (std::find(memberIds_.begin(), memberIds_.end(), id) != memberIds_.end())
    {
        throw std::runtime_error("Unit with id " + std::to_string(id) + " is already in the team.");
    }

    memberIds_.push_back(id);
}

void Team::removeMember(int id)
{
    memberIds_.erase(std::remove(memberIds_.begin(), memberIds_.end(), id),
                     memberIds_.end());
}

void Team::purgeDeadMembers(const Roster &roster)
{
    memberIds_.erase(std::remove_if(memberIds_.begin(), memberIds_.end(),
                                    [&roster](int id)
                                    { return !roster.contains(id); }),
                     memberIds_.end());
}

// Display -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Team::printTeam(const Roster &roster) const
{
    if (memberIds_.empty())
    {
        std::cout << "The team is empty." << std::endl;
        return;
    }

    std::cout << "Team (" << memberIds_.size() << " members):" << std::endl;
    for (int id : memberIds_)
    {
        const Unit &unit = roster.findUnitById(id);
        const Stats s = unit.getStats();
        std::cout << "  [" << unit.getId() << "] " << unit.getName()
                  << " (" << unit.getRace() << "*)"
                  << " - Lv " << unit.getLevel()
                  << " - HP " << s.getHealth() << "/" << s.getMaxHealth()
                  << " - XP " << unit.getExperience() << std::endl;

        if (!unit.getHook().empty())
            std::cout << "        " << unit.getHook() << std::endl;
    }
}
