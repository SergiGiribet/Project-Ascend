#include "Unit.h"

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
void Unit::addExperience(int exp) { experience += exp; }

void Unit::addSkill(const std::string &skill) { skills.push_back(skill); }

void Unit::takeDamage(int damage) { stats.decreaseHealth(damage); }

void Unit::heal(int amount) { stats.increaseHealth(amount); }

bool Unit::isAlive() const { return stats.isAlive(); }

// Display --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Unit::printUnit() const
{
    std::cout << "Unit ID: " << id << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Race: " << race << "*" << std::endl; //"★"
    std::cout << "Level: " << level << std::endl;
    stats.printStats();
    std::cout << "Experience: " << experience << std::endl;
    std::cout << "Skills: ";
    for (const auto &skill : skills)
    {
        std::cout << skill << " ";
    }
    std::cout << std::endl;
    std::cout << "History: " << history << std::endl;
    std::cout << "Hook: " << hook << std::endl;
}

