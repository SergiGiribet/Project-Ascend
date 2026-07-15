#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <map>
#include <random>
#include <vector>
#include "Unit.h"


class Generator {
    public:
        // Constructor -----------------------------------------------------------
        Generator(const std::string &resourcesDir);
        // Pre: resourcesDir must contain resources .txt files.
        // Post: Loads all banks and seeds the random engine.

        // Modifiers -------------------------------------------------------------
        Unit generateUnit(int id);
        // Pre: id must be a unique identifier.
        // Post: Returns a randomly generated unit: name, 1-2 distinct traits, weighted race, stats derived from race, and a filled backstory.

    private:
        std::map<std::string, std::vector<std::string>> banks_;
        std::mt19937 rng_;

        std::string pickRandom(const std::vector<std::string> &bank);
        std::string fillTemplate(const std:: string &tmpl);
};

#endif