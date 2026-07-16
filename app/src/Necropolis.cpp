#include "Necropolis.h"

#include <iostream>

Necropolis::Necropolis() : records_() {}

void Necropolis::addDeath(const Unit &unit, int floor, const std::string &cause, int turn)
{
    records_.push_back({unit.getName(), floor, cause, turn, unit.getSkills()});
}

bool Necropolis::empty() const
{
    return records_.empty();
}

const DeathRecord &Necropolis::pickRandom(std::mt19937 &rng) const
{
    if (records_.empty())
        throw std::runtime_error("The necropolis is empty.");
    std::uniform_int_distribution<size_t> dist(0, records_.size() - 1);
    return records_[dist(rng)];
}

void Necropolis::print() const
{
    if (records_.empty())
    {
        std::cout << "The Necropolis is empty. No one has fallen... yet." << std::endl;
        return;
    }

    std::cout << "=== The Necropolis (" << records_.size() << " fallen) ===" << std::endl;
    for (const DeathRecord &r : records_)
    {
        std::cout << r.name;

        if (!r.skills.empty())
        {
            std::cout << ", ";
            bool first = true;
            for (const std::string &skill : r.skills)
            {
                if (!first)
                    std::cout << " and ";
                std::cout << skill;
                first = false;
            }
        }

        std::cout << " - fell on floor " << r.floorDied
                  << " (incursion " << r.turn << "), " << r.cause << "." << std::endl;
    }
}