#include "Barracks.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>

int Barracks::create(const std::string &name)
{
    teams_.emplace_back(name);
    return static_cast<int>(teams_.size()) - 1;
}

int Barracks::count() const
{
    return static_cast<int>(teams_.size());
}

Team &Barracks::at(int index)
{
    return teams_.at(static_cast<size_t>(index));
}

const Team &Barracks::at(int index) const
{
    return teams_.at(static_cast<size_t>(index));
}

int Barracks::teamOfUnit(int unitId) const
{
    for (size_t i = 0; i < teams_.size(); i++)
    {
        const std::vector<int> &ids = teams_[i].getMembersIds();
        if (std::find(ids.begin(), ids.end(), unitId) != ids.end())
            return static_cast<int>(i);
    }
    return -1;
}

void Barracks::release(int unitId)
{
    int index = teamOfUnit(unitId);
    if (index != -1)
        at(index).removeMember(unitId);
}

std::map<int, std::string> Barracks::memberTags() const
{
    std::map<int, std::string> tags;
    for (const Team &t : teams_)
    {
        for (int id : t.getMembersIds())
        {
            tags[id] = t.getName();
        }
    }
    return tags;
}

void Barracks::purgeDead(const Roster &roster)
{
    for (Team &t : teams_)
    {
        t.purgeDeadMembers(roster);
    }
}

void Barracks::disband(int index)
{
    if (index < 0 || index >= count())
        throw std::out_of_range("There is no party " + std::to_string(index + 1) + ".");
    teams_.erase(teams_.begin() + index);
}

void Barracks::printAll(const Roster &roster) const
{
    if (teams_.empty())
    {
        std::cout << "No parties formed yet." << std::endl;
        return;
    }
    for (size_t i = 0; i < teams_.size(); i++)
    {
        std::cout << i + 1 << ". ";
        teams_[i].printTeam(roster);
    }
}

void Barracks::assign(int unitId, int index, const Roster &roster)
{
    if (teamOfUnit(unitId) == index)
        return;
    if (at(index).getMembersIds().size() >= static_cast<size_t>(Team::MAX_MEMBERS))
        throw std::runtime_error(at(index).getName() + " is full (" + std::to_string(Team::MAX_MEMBERS) + " members).");
    release(unitId);
    at(index).addMember(unitId, roster);
}
