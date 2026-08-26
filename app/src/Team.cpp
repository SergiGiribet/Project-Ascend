#include "Team.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

// Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
Team::Team(const std::string &name) : memberIds_(), name_(name) {}

// Consultors --------------------------------------------------------------------------------------------------------------------------------------------------------------------
const std::vector<int> &Team::getMembersIds() const
{
    return memberIds_;
}

const std::string &Team::getName() const
{
    return name_;
}

// Modifiers ---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Team::addMember(int id, const Roster &roster)
{
    if (memberIds_.size() >= static_cast<size_t>(MAX_MEMBERS))
        throw std::runtime_error("The team is full (" + std::to_string(MAX_MEMBERS) + " members).");

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
        std::cout << name_ << " (0/" << MAX_MEMBERS << "): no one assigned." << std::endl;
        return;
    }

    std::cout << name_ << " (" << memberIds_.size() << "/" << MAX_MEMBERS << "):" << std::endl;
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
