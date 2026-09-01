#include "Objective.h"
#include <array>


struct TraitFit
{
    const char *trait;
    std::array<int, 4> weight; // indexed by ObjectiveType: Slay, Hold, Retrieve, Rescue
};

// How well each trait serves each objective type (+-8 strong, +-4 mild, 0 irrelevant).
// Columns are deliberately near-zero-sum, so no objective type is secretly easier than
// another; intentional difficulty differences belong in makeObjective, where they show.
static const std::vector<TraitFit> TRAIT_FIT = {
    // trait            Slay    Hold    Retr    Resc
    {"Brave",         {   8,      8,      0,      4}},
    {"Cowardly",      {  -8,     -8,      4,     -4}},
    {"Reckless",      {   8,     -8,     -8,     -4}},
    {"Alert",         {   4,      4,      8,      4}},
    {"Boaster",       {   4,     -4,     -8,      0}},
    {"Curious",       {   0,     -4,      8,      0}},
    {"Stubborn",      {   0,      8,     -4,      0}},
    {"Loyal",         {   0,      4,      0,      8}},
    {"Gentle",        {  -8,      0,      0,      8}},
    {"Spiteful",      {   4,      0,      0,     -8}},
    {"Greedy",        {   0,     -4,      8,     -8}},
    {"Superstitious", {  -8,      4,      0,      0}},
};

static const int FLOOR_BASE = 20;  // what floor zero would be worth
static const int FLOOR_STEP = 15;  // what each floor adds
static const int FLOOR_SWING = 2;  // in FLOORS: how far a floor may sit from its own number

Objective makeObjective(int floor, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> pickType(0, 3);
    ObjectiveType type = static_cast<ObjectiveType>(pickType(rng));

    Objective objective;
    objective.type = type;
    std::uniform_int_distribution<int> swing(-FLOOR_STEP * FLOOR_SWING, FLOOR_STEP * FLOOR_SWING);
    objective.difficulty = FLOOR_BASE + floor * FLOOR_STEP + swing(rng);
    objective.rounds = 0;

    if (type == ObjectiveType::Hold)
        objective.rounds = 2 + floor / 4;

    return objective;
}

std::string describeObjective(const Objective &objective)
{
    switch (objective.type)
    {
    case ObjectiveType::Slay:
        return "Cut down what holds the floor.";
    case ObjectiveType::Hold:
        return "Hold the line for " + std::to_string(objective.rounds) + " rounds.";
    case ObjectiveType::Retrieve:
        return "Find what the floor hides and carry it out.";
    case ObjectiveType::Rescue:
        return "Free the captive before the floor claims them.";
    }
    return "Clear the floor.";
}

int traitFit(ObjectiveType type, const std::vector<std::string> &traits)
{
    int fit = 0;
    for (const std::string &trait : traits)
        for (const TraitFit &entry : TRAIT_FIT)
            if (trait == entry.trait)
                fit += entry.weight.at(static_cast<size_t>(type));
    return fit;
}

std::string describeFit(int fit)
{
    std::string amount = " (" + std::string(fit >= 0 ? "+" : "") + std::to_string(fit) + ").";

    if (fit >= 24)  return "This team was made for this" + amount;
    if (fit >= 8)   return "This team is well suited to it" + amount;
    if (fit > -8)   return "This team has no edge here" + amount;
    if (fit > -24)  return "This team is poorly suited to it" + amount;
    return "This team is badly wrong for this" + amount;
}
