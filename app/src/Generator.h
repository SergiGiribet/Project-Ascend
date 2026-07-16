#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <map>
#include <random>
#include <vector>
#include "Unit.h"
#include "Necropolis.h"

class Generator {
    public:
        // Constructor -----------------------------------------------------------
        Generator(const std::string &resourcesDir, std::mt19937 &rng);
        // Pre: resourcesDir must contain the seven resource .txt files (names, traits, jobs,
        //      places, motivations, templates, hooks), each with at least one entry (throws
        //      std::runtime_error otherwise); rng must outlive this Generator.
        // Post: Loads all banks and binds the generator to the provided random engine.


        // Modifiers -------------------------------------------------------------
        Unit generateUnit(int id, const Necropolis &necropolis);
        // Pre: id must be a unique identifier.
        // Post: Returns a randomly generated unit: name, 1-2 distinct traits, weighted race,
        //       stats derived from race (Reckless units are born with +30 max HP and +5 CON),
        //       and a filled backstory. If necropolis is not empty, the unit has a 40% chance
        //       of carrying a hook that references a fallen unit.

    private:
        std::map<std::string, std::vector<std::string>> banks_;
        std::mt19937 &rng_;

        std::string pickRandom(const std::vector<std::string> &bank);
        // Pre: bank must not be empty.
        // Post: Returns a uniformly random element of bank.

        std::string fillTemplate(const std::string &tmpl);
        // Pre: Every {placeholder} in tmpl must name an existing bank (throws
        //      std::out_of_range otherwise).
        // Post: Returns tmpl with every {placeholder} replaced by a random entry of its bank.
};

#endif