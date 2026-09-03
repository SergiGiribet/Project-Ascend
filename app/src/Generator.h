#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <map>
#include <random>
#include <vector>
#include <array>

#include "Unit.h"
#include "Necropolis.h"


// What you get for what you pay, when calling something out of the circle. Price buys the
// CEILING as well as the odds: a zero weight means that race simply CANNOT come out of that tier,
// so a cheap summoning can never produce a race 6 however lucky you are. Paying more buys a
// longer tail, never a guarantee -- the dearest tier can still answer with a race 1.
//
// Price and weights live in one struct on purpose. Apart they drift: someone changes what a tier
// costs, the odds stay where they were, and both numbers look reasonable on their own.
struct SummonTier {
    const char *name;               // what the menu calls it
    int price;                      // essence
    std::array<int, 6> raceWeights; // races 1..6, in order
};

// Mean race 1.7 / 2.9 / 4.3, which is a party of five worth roughly 114 / 138 / 165 power --
// steps of about two floors each. The cheap tier walls out around floor 6, and getting past that
// is what the essence is FOR.
static const std::array<SummonTier, 3> SUMMON_TIERS = {{
    {"whoever answers",     5, {40, 25, 15,  0,  0,  0}},
    {"a careful summoning", 20, {20, 20, 25, 20, 15,  0}},
    {"a name worth calling",60, { 5, 10, 15, 20, 25, 25}},
}};

class Generator {
    public:
        // Constructor -----------------------------------------------------------
        Generator(const std::string &resourcesDir, std::mt19937 &rng);
        // Pre: resourcesDir must contain the seven resource .txt files (names, traits, jobs,
        //      places, motivations, templates, hooks), each with at least one entry (throws
        //      std::runtime_error otherwise); rng must outlive this Generator.
        // Post: Loads all banks and binds the generator to the provided random engine.


        // Modifiers -------------------------------------------------------------
        Unit generateUnit(int id, const Necropolis &necropolis, const SummonTier &tier);
        // Pre: id must be a unique identifier; tier.raceWeights must not be all zeroes.
        // Post: Returns a randomly generated unit: full name, 1-2 distinct traits, a race drawn
        //       against tier.raceWeights (so the tier decides both the odds and the ceiling),
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
