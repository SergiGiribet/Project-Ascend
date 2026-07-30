#include "Objective.h"

Objective makeObjective(int floor, std::mt19937 &rng) {
    std::uniform_int_distribution<int> pickType(0, 3);
    ObjectiveType type = static_cast<ObjectiveType>(pickType(rng));

    Objective objective;
    objective.type = type;
    objective.difficulty = 20 + floor * 15;     // same magnitude Phase 1 called `danger`
    objective.rounds = 0;

    if (type == ObjectiveType::Hold)
        objective.rounds = 2 + floor / 4;

    return objective;
}

std::string describeObjective(const Objective &objective) {
    switch (objective.type)
    {
    case ObjectiveType::Slay:
        return "Cut down what holds the floor.";
        break;
    case ObjectiveType::Hold:
        return "Hold the line for " + std::to_string(objective.rounds) + " rounds.";
    case ObjectiveType::Retrieve:
        return "Find what the floor hides and carry it out.";
    case ObjectiveType::Rescue:
        return "Free the captive before the floor claims them.";
    }
    return "Clear the floor.";
}