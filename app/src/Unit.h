#ifndef UNIT_H
#define UNIT_H

#include <string>
#include <vector>

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
    // Post: Initializes the Stats object with default values.

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
    // Post: Prints the current stats of the unit to the console.

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
    std::string history;
    std::string hook;

public:

    // Constructor -------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Unit(int id);
    // Pre: id must be a unique identifier for the unit.
    // Post: Initializes the Unit object with default values for name, race, level, stats, experience.

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
    // Post: Returns the Stats object associated with the unit.

    int getExperience() const;
    // Pre: None
    // Post: Returns the experience points of the unit.

    const std::vector<std::string> &getSkills() const;
    // Pre: None
    // Post: Returns the list of skills of the unit.

    std::string getHistory() const;
    // Pre: None
    // Post: Returns the history (backstory) of the unit.

    std::string getHook() const;
    // Pre: None
    // Post: Returns the hook of the unit (empty string if it has none).

    // Setters ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void setName(const std::string &n);
    // Pre: n must be a valid string.
    // Post: Sets the name of the unit to n.

    void setRace(int r);
    // Pre: r must be a valid race identifier
    // Post: Sets the race of the unit to r.

    void setLevel(int l);
    // Pre: l must be a positive integer.
    // Post: Sets the level of the unit to l.

    void setStats(const Stats &s);
    // Pre : s must be a valid Stats object.
    // Post: Sets the Stats object of the unit to s.

    void setExperience(int exp);
    // Pre: exp must be a non-negative integer.
    // Post: Sets the experience points of the unit to exp.

    void setHook(const std::string &h);
    // Pre: h must be a valid string.
    // Post: Sets the hook of the unit to h.

    void setHistory(const std::string &h);
    // Pre: h must be a valid string.
    // Post: Sets the history of the unit to h.

    // Modifiers ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void addExperience(int exp);
    // Pre: exp must be a non-negative integer.
    // Post: Increases the experience points of the unit by exp.

    void addSkill(const std::string &skill);
    // Pre: skill must be a valid string.
    // Post: Adds skill to the unit's list of skills.

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
    // Post: Prints the details of the unit.

};

#endif
