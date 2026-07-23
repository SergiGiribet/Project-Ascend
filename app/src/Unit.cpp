#include "Unit.h"
#include "Utils.h"

#include <iostream>
#include <algorithm>

// Stats
// Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
Stats::Stats()
    : health(100), maxHealth(100), strength(10), constitution(10) {}

Stats::Stats(int health, int maxHealth, int strength, int constitution)
    : health(health), maxHealth(maxHealth), strength(strength), constitution(constitution) {}

// Getters ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int Stats::getHealth() const { return health; }

int Stats::getMaxHealth() const { return maxHealth; }

int Stats::getStrength() const { return strength; }

int Stats::getConstitution() const { return constitution; }

// Setters --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Stats::setHealth(int h) { health = h; }
void Stats::setMaxHealth(int mh) { maxHealth = mh; }

void Stats::setStrength(int s) { strength = s; }

void Stats::setConstitution(int c) { constitution = c; }

// Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Stats::increaseHealth(int amount) { health = std::min(health + amount, maxHealth); }

void Stats::decreaseHealth(int amount) { health = std::max(health - amount, 0); }

bool Stats::isAlive() const { return health > 0; }

// Display --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Stats::printStats() const
{
    std::cout << "Health: " << health << "/" << maxHealth << std::endl;
    std::cout << "Strength: " << strength << std::endl;
    std::cout << "Constitution: " << constitution << std::endl;
}


// Unit
// Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
Unit::Unit(int id)
    : id(id), experience(0), stats(), name("John Doe"), race(0), level(1) {}

Unit::Unit(int id, const std::string &name, int race, int level, int health, int maxHealth, int strength, int constitution)
    : id(id), experience(0), stats(health, maxHealth, strength, constitution), name(name), race(race), level(level) {}

// Getters ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int Unit::getId() const { return id; }

std::string Unit::getName() const { return name; }

int Unit::getRace() const { return race; }

int Unit::getLevel() const { return level; }

Stats Unit::getStats() const { return stats; }

int Unit::getExperience() const { return experience; }

const std::vector<std::string> &Unit::getSkills() const { return skills; }

std::string Unit::getHistory() const { return history; }

std::string Unit::getHook() const { return hook; }

// Setters ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Unit::setName(const std::string &n) { name = n; }

void Unit::setRace(int r) { race = r; }

void Unit::setLevel(int l) { level = l; }

void Unit::setStats(const Stats &s) { stats = s; }

void Unit::setExperience(int exp) { experience = exp; }

void Unit::setHook(const std::string &h) { hook = h; }

void Unit::setHistory(const std::string &h) { history = h; }

// Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int Unit::addExperience(int exp) { 
    experience += exp;
    int levelsGained = 0;
    while (experience >= level * 100) {
        experience -= level * 100;
        level++;
        levelsGained++;
        stats.setMaxHealth(stats.getMaxHealth() + 10);
        stats.setHealth(stats.getHealth() + 10);
        stats.setStrength(stats.getStrength() + 1);
        stats.setConstitution(stats.getConstitution() + 1);
    }
    return levelsGained;
}

void Unit::addSkill(const std::string &skill) { skills.push_back(skill); }

void Unit::removeSkill(const std::string &skill) {
    skills.erase(std::remove(skills.begin(), skills.end(), skill), skills.end());
}

void Unit::addInjury(const std::string &injury) { injuries.push_back(injury); }

const std::vector<std::string> &Unit::getInjuries() const { return injuries; }

void Unit::takeDamage(int damage) { stats.decreaseHealth(damage); }

void Unit::heal(int amount) { stats.increaseHealth(amount); }

bool Unit::isAlive() const { return stats.isAlive(); }

// Display --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Unit::printUnit() const
{
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "  " << name << " (" << race << "*)  [ID " << id << "]" << std::endl;
    std::cout << "  Level " << level << "  |  XP " << experience << std::endl;
    std::cout << "  HP " << stats.getHealth() << "/" << stats.getMaxHealth()
              << "  |  STR " << stats.getStrength()
              << "  |  CON " << stats.getConstitution() << std::endl;
    std::cout << "  Traits: ";
    bool first = true;
    for (const auto &skill : skills)
    {
        if (!first)
            std::cout << ", ";
        std::cout << skill;
        first = false;
    }
    std::cout << std::endl;
    if (!injuries.empty())
    {
        std::cout << COLOR_RED << "  Injuries: ";
        bool firstInjury = true;
        for (const auto &injury : injuries)
        {
            if (!firstInjury)
                std::cout << ", ";
            std::cout << injury;
            firstInjury = false;
        }
        std::cout << COLOR_RESET << std::endl;
    }
    std::cout << "  Story:  " << history << std::endl;
    if (!hook.empty())
        std::cout << "  Hook:   " << hook << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

