#include "Roster.h"
#include "Utils.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

// Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------

Roster::Roster() : units_() {}

// Consultors ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Unit &Roster::findUnitById(int id)
{
    for (auto &unit : units_)
    {
        if (unit.getId() == id)
        {
            return unit;
        }
    }
    throw std::runtime_error("Unit with id " + std::to_string(id) + " not found.");
}

const Unit &Roster::findUnitById(int id) const
{
    for (const auto &unit : units_)
    {
        if (unit.getId() == id)
        {
            return unit;
        }
    }
    throw std::runtime_error("Unit with id " + std::to_string(id) + " not found.");
}

bool Roster::contains(int id) const
{
    for (const auto &unit : units_)
    {
        if (unit.getId() == id)
        {
            return true;
        }
    }
    return false;
}

// Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Roster::addUnit(const Unit &unit)
{
    units_.push_back(unit);
}

void Roster::removeUnitById(int id)
{
    units_.erase(std::remove_if(units_.begin(), units_.end(),
                                [id](const Unit &unit)
                                { return unit.getId() == id; }),
                 units_.end());
}


void Roster::healRested(const std::vector<int> &climbedIds) {
    for (Unit &u : units_) {
        if (std::find(climbedIds.begin(), climbedIds.end(), u.getId()) == climbedIds.end()) {
            u.heal(u.getStats().getMaxHealth());
        }
    }
}

// Display ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Roster::printRoster(const std::map<int, std::string> &partyTags, const std::vector<int> &trainerIds, const std::vector<int> &traineeIds) const
{
    if (units_.empty())
    {
        std::cout << "The roster is empty. Invoke someone." << std::endl;
        return;
    }

    std::cout << "Roster (" << units_.size() << " units):" << std::endl;
    for (const auto &unit : units_)
    {
        std::map<int, std::string>::const_iterator party = partyTags.find(unit.getId());
        bool isTrainer = std::find(trainerIds.begin(), trainerIds.end(), unit.getId()) != trainerIds.end();
        bool isTrainee = std::find(traineeIds.begin(), traineeIds.end(), unit.getId()) != traineeIds.end();

        std::string tag;
        if (party != partyTags.end())
        {
            std::cout << COLOR_CYAN;
            tag = " [" + party->second + "]";
        }
        else if (isTrainer)
        {
            std::cout << COLOR_MAGENTA;
            tag = " [trainer]";
        }
        else if (isTrainee)
        {
            std::cout << COLOR_BLUE;
            tag = " [trainee]";
        }

        const Stats s = unit.getStats();
        std::cout << "  [" << unit.getId() << "] " << unit.getName()
                  << " (" << unit.getRace() << "*)"
                  << " - Lv " << unit.getLevel()
                  << " - HP " << s.getHealth() << "/" << s.getMaxHealth()
                  << " - XP " << unit.getExperience();
        if (!tag.empty())
            std::cout << tag << COLOR_RESET;

        const std::vector<Injury> &inj = unit.getInjuries();
        if (!inj.empty())
        {
            std::cout << " " << COLOR_RED << "{";
            bool firstInjury = true;
            for (const Injury &injury : inj)
            {
                if (!firstInjury)
                    std::cout << ", ";
                std::cout << injuryLabel(injury);
                firstInjury = false;
            }
            std::cout << "}" << COLOR_RESET;
        }
        std::cout << std::endl;
    }
}
