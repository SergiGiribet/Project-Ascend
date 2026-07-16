#include "Roster.h"

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
                                [id](const Unit &unit) { return unit.getId() == id; }),
                 units_.end());
}

// Display ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Roster::printRoster() const
{
    if (units_.empty())
    {
        std::cout << "The roster is empty. Invoke someone." << std::endl;
        return;
    }

    std::cout << "Roster (" << units_.size() << " units):" << std::endl;
    for (const auto &unit : units_)
    {
        const Stats s = unit.getStats();
        std::cout << "  [" << unit.getId() << "] " << unit.getName()
                  << " (" << unit.getRace() << "*)"
                  << " - Lv " << unit.getLevel()
                  << " - HP " << s.getHealth() << "/" << s.getMaxHealth()
                  << " - XP " << unit.getExperience() << std::endl;
    }
}
