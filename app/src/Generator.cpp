#include "Generator.h"

#include <fstream>
#include <stdexcept>
#include <algorithm>

static std::vector<std::string> loadBank(const std::string &path) {
    std::vector<std::string> bank;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) 
        if (!line.empty()) bank.push_back(line);
    return bank;
}

// Constructor -------------------------------------------------------------------
Generator::Generator(const std::string &resourcesDir, std::mt19937 &rng) : banks_(), rng_(rng) {
        banks_["name"] = loadBank(resourcesDir + "/names.txt");
        banks_["job"] = loadBank(resourcesDir + "/jobs.txt");
        banks_["motivation"] = loadBank(resourcesDir + "/motivations.txt");
        banks_["place"] = loadBank(resourcesDir + "/places.txt");
        banks_["trait"] = loadBank(resourcesDir + "/traits.txt");
        banks_["template"] = loadBank(resourcesDir + "/templates.txt");
        banks_["hook"] = loadBank(resourcesDir + "/hooks.txt");

        for (const auto &pair : banks_)
            if (pair.second.empty())
                throw std::runtime_error("Bank '" + pair.first + "' is empty or file not found.");
    }

// Modifiers ---------------------------------------------------------------------
std::string Generator::pickRandom(const std::vector<std::string> &bank) {
    std::uniform_int_distribution<size_t> dist(0, bank.size() -1);
    return bank[dist(rng_)];
}

std::string Generator::fillTemplate(const std::string &tmpl) {
    std::string result = tmpl; // copy to work
    size_t open = result.find('{');
    while (open != std::string::npos) {
        size_t close = result.find('}', open);
        std::string key = result.substr(open +1, close - open -1);
        result.replace(open, close - open +1, pickRandom(banks_.at(key)));
        open = result.find('{');
    }
    return result;
}

Unit Generator::generateUnit(int id, const Necropolis &necropolis) {
    Unit unit(id);
    unit.setName(pickRandom(banks_.at("name")));
    std::string t1 = pickRandom(banks_.at("trait"));
    unit.addSkill(t1);
    std::uniform_int_distribution<int> coin(0,1);
    if (coin(rng_) == 1) {
        std::string t2 = pickRandom(banks_.at("trait"));
        while (t2 == t1) t2 = pickRandom(banks_.at("trait"));
        unit.addSkill(t2);
    }
    std::discrete_distribution<int> raceDist{40, 25, 15, 10, 7, 3};
    int race = raceDist(rng_)+1; // this returns 0-5 +1 for the 1-6*
    unit.setRace(race);
    Stats stats(80 + race * 20, 80 + race * 20, 8 + race * 2, 8 + race * 2);
    const std::vector<std::string> &skills = unit.getSkills();
    if (std::find(skills.begin(), skills.end(), "Reckless") != skills.end()) {
        stats.setMaxHealth(stats.getMaxHealth() + 30);
        stats.setHealth(stats.getHealth() + 30);
        stats.setConstitution(stats.getConstitution() + 5);
    }
    unit.setStats(stats);
    unit.setHistory(fillTemplate(pickRandom(banks_.at("template"))));

    if (!necropolis.empty()) {
        std::uniform_int_distribution<int> chance(1, 100);
        if (chance(rng_) <= 40) {
            const DeathRecord &fallen = necropolis.pickRandom(rng_);
            std::string hook = pickRandom(banks_.at("hook"));
            size_t pos = hook.find("{fallen}");
            if (pos != std::string::npos)
                hook.replace(pos, 8, fallen.name);
            unit.setHook(hook + ".");
        }
    }
    return unit;
}