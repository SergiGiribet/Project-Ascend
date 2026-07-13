#include "Roster.h"

#include <algorithm>
#include <stdexcept>

// Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------

Roster::Roster() : units_() {}

// Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Roster::addUnit(const Unit &unit)
{
    units_.push_back(unit);
}

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

void Roster::removeUnitById(int id)
{
    units_.erase(std::remove_if(units_.begin(), units_.end(),
                                [id](const Unit &unit) { return unit.getId() == id; }),
                 units_.end());
}

void Roster::printRoster() const
{
    std::cout << "Roster:" << std::endl;
    for (const auto &unit : units_)
    {
        std::cout << "ID: " << unit.getId() << ", Name: " << unit.getName() << ", Race: " << unit.getRace() << ", Level: " << unit.getLevel() << std::endl;
    }
}