#include "Injury.h"

#include <fstream>
#include <algorithm>
#include <stdexcept>

std::vector<Injury> loadInjuries(const std::string &path) {
    std::vector<Injury> injuries;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty())
            continue;
        
        size_t firstPipe = line.find('|');
        if (firstPipe == std::string::npos)
            throw std::runtime_error("Malformed line in " + path + ": " + line);
            
        size_t secondPipe = line.find('|', firstPipe + 1);
        if (secondPipe == std::string::npos)
            throw std::runtime_error("Malformed line in " + path + ": " + line);
        
        std::string name = line.substr(0, firstPipe);
        std::string strStr = line.substr(firstPipe + 1, secondPipe - firstPipe -1);
        std::string conStr = line.substr(secondPipe + 1);

        injuries.push_back({name, std::stoi(strStr), std::stoi(conStr)});
    }
    if (injuries.empty())
        throw std::runtime_error("Injury bank is empty or file not found: " + path);
    return injuries;

}

void applyInjury(Unit &unit, const std::vector<Injury> &bank, std::mt19937 &rng) {
    std::uniform_int_distribution<size_t> pick(0, bank.size() -1);
    const Injury &injury = bank[pick(rng)];

    Stats s = unit.getStats();
    s.setStrength(std::max(1, s.getStrength()- injury.strPenalty));
    s.setConstitution(std::max(1, s.getConstitution() - injury.conPenalty));
    unit.setStats(s);

    unit.addSkill(injury.name);

    const std::vector<std::string> &skills = unit.getSkills();
    bool hasReckless = std::find(skills.begin(), skills.end(), "Reckless") != skills.end();
    bool hasBoaster = std::find(skills.begin(), skills.end(), "Boaster") != skills.end();

    if (hasReckless || hasBoaster) {
        std::uniform_int_distribution<int> coin(0, 1);
        if (coin(rng) == 1) {
            if (hasReckless) unit.removeSkill("Reckless");
            if (hasBoaster) unit.removeSkill("Boaster");

            if (std::find(skills.begin(), skills.end(), "Alert") == skills.end())
                unit.addSkill("Alert");
        }
    }
}