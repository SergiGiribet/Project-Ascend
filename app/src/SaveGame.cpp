#include "SaveGame.h"

#include <fstream>
#include <stdexcept>
#include <sstream>

// A list on one line, comma separated, written only if there is anything to write -- an
// absent key means the default, and a unit with no traits simply has no skills line.
static void writeList(std::ofstream &out, const std::string &key,
                      const std::vector<std::string> &items)
{
    if (items.empty())
        return;

    out << key << " ";
    bool first = true;
    for (const std::string &item : items)
    {
        if (!first)
            out << ",";
        out << item;
        first = false;
    }
    out << "\n";
}

void saveGame(const std::string &path, const GameState &state, const Roster &roster,
              const Barracks &barracks)
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

        if (!u.getHook().empty())
            out << "hook " << u.getHook() << "\n";

        writeList(out, "skills", u.getSkills());

        for (const Injury &inj : u.getInjuries())
            out << "injury " << inj.name << "," << inj.strPenalty << "," << inj.conPenalty << "\n";
    }

    for (const DeathRecord &d : state.necropolis.records())
    {
        out << "\n[death]\n";
        out << "name " << d.name << "\n";
        out << "floor " << d.floorDied << "\n";
        out << "cause " << d.cause << "\n";
        out << "turn " << d.turn << "\n";
        writeList(out, "skills", d.skills);
    }

    for (const auto &[n, obj] : state.floorObjectives)
    {
        out << "\n[floor]\n";
        out << "n " << n << "\n";
        out << "type " << static_cast<int>(obj.type) << "\n";
        out << "difficulty " << obj.difficulty << "\n";
        out << "rounds " << obj.rounds << "\n"; 
    }

    for (const auto &[n, r] : state.floorReports)
    {
        out << "\n[report]\n";
        out << "n " << n << "\n";
        if (!r.scout.empty())
            out << "scout " << r.scout << "\n";
        out << "scoutId " << r.scoutId << "\n";
        out << "sawObjective " << (r.sawObjective ? 1 : 0) << "\n";
        out << "sawDanger " << (r.sawDanger ? 1 : 0) << "\n";
        out << "bias " << r.bias << "\n";
        out << "claimedType " << static_cast<int>(r.claimed.type) << "\n";
        out << "claimedDifficulty " << r.claimed.difficulty << "\n";
        out << "claimedRounds " << r.claimed.rounds << "\n";
    }


    // Last on purpose: a party is a list of ids, so every one of them has to be back on the
    // roster before the party that names them is read.
    for (int i = 0; i < barracks.count(); ++i)
    {
        const Team &t = barracks.at(i);
        out << "\n[party]\n";
        out << "name " << t.getName() << "\n";
        std::vector<std::string> ids;
        for (int id : t.getMembersIds())
            ids.push_back(std::to_string(id));
        writeList(out, "members", ids);
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

struct PendingDeath
{
    std::string name, cause;
    int floor = 0, turn = 0;
    std::vector<std::string> skills;
};

// A party is read as a name and a list of ids, and only turned into a real one once the
// whole block has arrived -- Barracks::assign wants the roster to already hold each member.
struct PendingParty
{
    std::string name;
    std::vector<int> members;
};

struct PendingFloor
{
    int n = 0, type = 0, difficulty = 0, rounds = 0;
};

struct PendingReport
{
    int n = 0, scoutId = -1, bias = 0, claimedType = 0, claimedDifficulty = 0, claimedRounds = 0;
    std::string scout;
    bool sawObjective = false, sawDanger = false;
};

static ObjectiveType toType(int t)
{
    if (t < 0 || t > 3)
        throw std::runtime_error("Save file has an unknown objective type: " + std::to_string(t));
    return static_cast<ObjectiveType>(t);
}

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

static DeathRecord buildDeath(const PendingDeath &p)
{
    return DeathRecord{p.name, p.floor, p.cause, p.turn, p.skills};
}

static Report buildReport(const PendingReport &p)
{
    Report r;
    r.scout = p.scout;
    r.scoutId = p.scoutId;
    r.sawObjective = p.sawObjective;
    r.sawDanger = p.sawDanger;
    r.bias = p.bias;
    r.claimed = Objective{toType(p.claimedType), p.claimedDifficulty, p.claimedRounds};
    return r;
}

bool loadGame(const std::string &path, GameState &state, Roster &roster,
              Barracks &barracks)
{
    std::ifstream in(path);
    if (!in)
        return false;

    PendingUnit pending;
    PendingDeath dying;
    PendingParty onParty;
    PendingFloor onFloor;
    PendingReport onReport;
    std::string section;

    auto closeSection = [&]()
    {
        if (section == "unit")
            roster.addUnit(buildUnit(pending));
        else if (section == "death")
            state.necropolis.addRecord(buildDeath(dying));
        else if (section == "floor")
            state.floorObjectives[onFloor.n] = Objective{toType(onFloor.type), onFloor.difficulty, onFloor.rounds};
        else if (section == "party")
        {
            int index = barracks.create(onParty.name);
            for (int id : onParty.members)
                if (roster.contains(id))
                    barracks.assign(id, index, roster);
        }
        else if (section == "report")
            state.floorReports[onReport.n] = buildReport(onReport);
        section.clear();
    };

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        if (line == "[unit]")
        {
            closeSection();
            section = "unit";
            pending = PendingUnit();
            continue;
        }

        if (line == "[death]")
        {
            closeSection();
            section = "death";
            dying = PendingDeath();
            continue;
        }

        if (line == "[floor]")
        {
            closeSection();
            section = "floor";
            onFloor = PendingFloor();
            continue;
        }

        if (line == "[party]")
        {
            closeSection();
            section = "party";
            onParty = PendingParty();
            continue;
        }

        if (line == "[report]")
        {
            closeSection();
            section = "report";
            onReport = PendingReport();
            continue;
        }

        size_t pos = line.find(' ');
        if (pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (section.empty())
        {
            if (key == "essence")               state.essence = std::stoi(value);
            else if (key == "highestFloor")     state.highestFloor = std::stoi(value);
            else if (key == "incursionCount")   state.incursionCount = std::stoi(value);
            else if (key == "nextUnitId")       state.nextUnitId = std::stoi(value);
            continue;
        }

        if (section == "unit")
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

        if (section == "death")
        {
            if (key == "name")          dying.name = value;
            else if (key == "cause")    dying.cause = value;
            else if (key == "floor")    dying.floor = std::stoi(value);
            else if (key == "turn")     dying.turn = std::stoi(value);
            else if (key == "skills")
            {
                std::istringstream ss(value);
                std::string item;
                while (std::getline(ss, item, ','))
                    dying.skills.push_back(item);
            }
            continue;
        }

        if (section == "party")
        {
            if (key == "name")
                onParty.name = value;
            else if (key == "members")
            {
                std::istringstream ss(value);
                std::string item;
                while (std::getline(ss, item, ','))
                    onParty.members.push_back(std::stoi(item));
            }
            continue;
        }

        if (section == "floor")
        {
            if (key == "n")                 onFloor.n = std::stoi(value);
            else if (key == "type")         onFloor.type = std::stoi(value);
            else if (key == "difficulty")   onFloor.difficulty = std::stoi(value);
            else if (key == "rounds")       onFloor.rounds = std::stoi(value);
            continue;
        }

        if (section == "report")
        {
            if (key == "n")                        onReport.n = std::stoi(value);
            else if (key == "scout")               onReport.scout = value;
            else if (key == "scoutId")             onReport.scoutId = std::stoi(value);
            else if (key == "sawObjective")        onReport.sawObjective = (std::stoi(value) != 0);
            else if (key == "sawDanger")           onReport.sawDanger = (std::stoi(value) != 0);
            else if (key == "bias")                onReport.bias = std::stoi(value);
            else if (key == "claimedType")         onReport.claimedType = std::stoi(value);
            else if (key == "claimedDifficulty")   onReport.claimedDifficulty = std::stoi(value);
            else if (key == "claimedRounds")       onReport.claimedRounds = std::stoi(value);
            continue;
        }

    }

    closeSection();
    return true;
}