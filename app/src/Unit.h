#ifndef UNIT_H
#define UNIT_H

#include <string>
#include <vector>
#include "Injury.h"

class Stats
{
private:
    int health;
    int maxHealth;
    int strength;
    int constitution;

public:
    // Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Stats();
    // Pre: None
    // Post: Initializes stats to 100/100 health, 10 strength and 10 constitution.

    Stats(int health, int maxHealth, int strength, int constitution);
    // Pre: All parameters must be non-negative.
    // Post: Initializes the Stats object with the provided values.

    // Getters ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    int getHealth() const;
    // Pre: None
    // Post: Returns the current health value.

    int getMaxHealth() const;
    // Pre: None
    // Post: Returns the maximum health value.

    int getStrength() const;
    // Pre: None
    // Post: Returns the strength value.

    int getConstitution() const;
    // Pre: None
    // Post: Returns the constitution value.

    // Setters --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void setHealth(int h);
    // Pre: h must be non-negative and less than or equal to maxHealth.
    // Post: Sets the current health value to h.

    void setMaxHealth(int mh);
    // Pre: mh must be non-negative and greater than or equal to health.
    // Post: Sets the maximum health value to mh.

    void setStrength(int s);
    // Pre: s must be non-negative.
    // Post: Sets the strength value to s.

    void setConstitution(int c);
    // Pre: c must be non-negative.
    // Post: Sets the constitution value to c.

    // Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void increaseHealth(int amount);
    // Pre: amount must be non-negative.
    // Post: Increases the current health value by amount, ensuring it does not exceed maxHealth.

    void decreaseHealth(int amount);
    // Pre: amount must be non-negative.
    // Post: Decreases the current health value by amount, ensuring it does not go below zero.

    bool isAlive() const;
    // Pre: None
    // Post: Returns true if the unit's health is greater than zero, indicating that the unit is alive; otherwise, returns false.

    // Display ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void printStats() const;
    // Pre: None
    // Post: Prints health, strength and constitution to the console, one per line.
};

class Unit
{
private:
    int id;
    int experience;

    Stats stats;

    std::string name;
    int race;
    int level;

    std::vector<std::string> skills;
    std::vector<Injury> injuries;
    std::string history;
    std::string hook;

public:
    // Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Unit(int id);
    // Pre: id must be a unique identifier for the unit.
    // Post: Initializes the unit at level 1 with 0 experience, default stats and a placeholder
    //       name; skills, history and hook start empty.

    Unit(int id, const std::string &name, int race, int level, int health, int maxHealth, int strength, int constitution);
    // Pre: id must be a unique identifier for the unit. health, maxHealth, strength, and constitution must be non-negative.
    // Post: Initializes the Unit object with the provided values.

    // Getters ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    int getId() const;
    // Pre: None
    // Post: Returns the unique identifier of the unit.

    std::string getName() const;
    // Pre: None
    // Post: Returns the name of the unit.

    int getRace() const;
    // Pre: None
    // Post: Returns the race of the unit.

    int getLevel() const;
    // Pre: None
    // Post: Returns the level of the unit.

    Stats getStats() const;
    // Pre: None
    // Post: Returns a COPY of the unit's stats (a read-only snapshot: modifying it does not
    //       affect the unit; use takeDamage/heal/addExperience to change the real stats).

    int getExperience() const;
    // Pre: None
    // Post: Returns the experience points of the unit.

    const std::vector<std::string> &getSkills() const;
    // Pre: None
    // Post: Returns a read-only reference to the unit's traits.

    const std::vector<Injury> &getInjuries() const;
    // Pre: None
    // Post: Returns a read-only reference to the unit's permanent injuries (kept apart from
    //       traits; each carries its own stat penalties and is never removed).

    std::string getHistory() const;
    // Pre: None
    // Post: Returns the history (backstory) of the unit.

    std::string getHook() const;
    // Pre: None
    // Post: Returns the hook of the unit (empty string if it has none).

    // Setters ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void setName(const std::string &n);
    // Pre: None
    // Post: Sets the name of the unit to n.

    void setRace(int r);
    // Pre: r must be a valid race identifier
    // Post: Sets the race of the unit to r.

    void setLevel(int l);
    // Pre: l must be a positive integer.
    // Post: Sets the level of the unit to l.

    void setStats(const Stats &s);
    // Pre: None
    // Post: Replaces the unit's stats with s.

    void setExperience(int exp);
    // Pre: exp must be a non-negative integer.
    // Post: Sets the experience points of the unit to exp.

    void setHook(const std::string &h);
    // Pre: None
    // Post: Sets the hook of the unit to h.

    void setHistory(const std::string &h);
    // Pre: None
    // Post: Sets the history of the unit to h.

    // Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    int addExperience(int exp);
    // Pre: exp must be a non-negative integer.
    // Post: Adds exp to the unit's experience; while experience >= level * 100, consumes that
    //       amount and levels up (+1 level, +10 max HP and HP, +1 STR, +1 CON).
    //       Returns the number of levels gained (0 if none).

    void addSkill(const std::string &skill);
    // Pre: None
    // Post: Appends skill to the unit's list of traits.

    void removeSkill(const std::string &skill);
    // Pre: None
    // Post: Removes the first occurrence of skill from the unit's list of traits; does nothing
    //       if the unit doesn't have it.

    void addInjury(const Injury &injury);
    // Pre: None
    // Post: Records injury in the unit's permanent injuries (kept apart from traits). The stat
    //       penalties are applied by applyInjury, not here -- this only stores the record.

    void takeDamage(int damage);
    // Pre: damage must be a non-negative integer.
    // Post: Decreases the unit's health by damage, ensuring it does not go below zero.

    void heal(int amount);
    // Pre: amount must be a non-negative integer.
    // Post: Increases the unit's health by amount, ensuring it does not exceed the maximum health.

    bool isAlive() const;
    // Pre: None
    // Post: Returns true if the unit's health is greater than zero, indicating that the unit is alive; otherwise, returns false.

    // Display ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void printUnit() const;
    // Pre: None
    // Post: Prints the unit card (name, race, level, XP, stats, traits, story and hook if any)
    //       framed by dashed lines. Permanent injuries, if any, print on a red "Injuries:" line
    //       with each injury's stat cost.
};

#endif
