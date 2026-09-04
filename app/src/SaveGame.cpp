#include "SaveGame.h"

#include <fstream>
#include <stdexcept>
#include <sstream>

void saveGame(const std::string &path, const GameState &state, const Roster &roster)
{
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Could not open save file: " + path);

    out << "version 1\n";
    out << "essence " << state.essence << "\n";
    out << "highestFloor " << state.highestFloor << "\n";
    out << "incursionCount " << state.incursionCount << "\n";
    out << "nextUnitId " << state.nextUnitId << "\n";

    for (const Unit &u : roster.units())
    {
        const Stats s = u.getStats();
        out << "\n[unit]\n";
        out << "id " << u.getId() << "\n";
        out << "name " << u.getName() << "\n";
        out << "race " << u.getRace() << "\n";
        out << "level " << u.getLevel() << "\n";
        out << "xp " << u.getExperience() << "\n";
        out << "hp " << s.getHealth() << "\n";
        out << "maxHp " << s.getMaxHealth() << "\n";
        out << "str " << s.getStrength() << "\n";
        out << "con " << s.getConstitution() << "\n";
        out << "history " << u.getHistory() << "\n";

        // An absent key means the default, so an empty value writes no line at all:
        // nothing to read back, and no trailing spaces left lying around the file.
        if (!u.getHook().empty())
            out << "hook " << u.getHook() << "\n";

        if (!u.getSkills().empty())
        {
            out << "skills ";
            bool first = true;
            for (const std::string &t : u.getSkills())
            {
                if (!first)
                    out << ",";
                out << t;
                first = false;
            }
            out << "\n";
        }

        for (const Injury &inj : u.getInjuries())
            out << "injury " << inj.name << "," << inj.strPenalty << "," << inj.conPenalty << "\n";
    }
}

// The fields of the unit currently being read. They arrive one per line, and the Unit itself
// cannot be built until the last of them has: the constructor wants the id, and the stats want
// four numbers that come on four different lines.
struct PendingUnit
{
    int id = 0, race = 1, level = 1, xp = 0;
    int hp = 100, maxHp = 100, str = 10, con = 10;
    std::string name, history, hook;
    std::vector<std::string> skills;
    std::vector<Injury> injuries;
};

static Unit buildUnit(const PendingUnit &p)
{
    Unit u(p.id);
    u.setName(p.name);
    u.setRace(p.race);
    u.setLevel(p.level);
    u.setExperience(p.xp);
    u.setStats(Stats(p.hp, p.maxHp, p.str, p.con));
    u.setHistory(p.history);
    u.setHook(p.hook);
    for (const std::string &t : p.skills)
        u.addSkill(t);
    for (const Injury &inj : p.injuries)
        u.addInjury(inj);
    return u;
}

bool loadGame(const std::string &path, GameState &state, Roster &roster)
{
    std::ifstream in(path);
    if (!in)
        return false;

    PendingUnit pending;
    bool building = false;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        if (line == "[unit]")
        {
            if (building)
                roster.addUnit(buildUnit(pending));
            pending = PendingUnit();
            building = true;
            continue;
        }

        size_t pos = line.find(' ');
        if (pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (!building)
        {
            if (key == "essence")
                state.essence = std::stoi(value);
            else if (key == "highestFloor")
                state.highestFloor = std::stoi(value);
            else if (key == "incursionCount")
                state.incursionCount = std::stoi(value);
            else if (key == "nextUnitId")
                state.nextUnitId = std::stoi(value);
            continue;
        }

        if (building)
        {
            if (key == "id")            pending.id = std::stoi(value);
            else if (key == "race")     pending.race = std::stoi(value);
            else if (key == "level")    pending.level = std::stoi(value);
            else if (key == "xp")       pending.xp = std::stoi(value);
            else if (key == "hp")       pending.hp = std::stoi(value);
            else if (key == "maxHp")    pending.maxHp = std::stoi(value);
            else if (key == "str")      pending.str = std::stoi(value);
            else if (key == "con")      pending.con = std::stoi(value);
            else if (key == "name")     pending.name = value;
            else if (key == "history")  pending.history = value;
            else if (key == "hook")     pending.hook = value;
            else if (key == "skills")
            {
                std::istringstream ss(value);
                std::string item;
                while (std::getline(ss, item, ','))
                    pending.skills.push_back(item);
            }
            else if (key == "injury")
            {
                std::istringstream ss(value);
                std::string name, sp, cp;
                if (std::getline(ss, name, ',') && std::getline(ss, sp, ',') && std::getline(ss, cp))
                    pending.injuries.push_back(Injury{name, std::stoi(sp), std::stoi(cp)});
            }
        }
    }

    if (building)
        roster.addUnit(buildUnit(pending));
    return true;
}