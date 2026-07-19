#include "Incursion.h"
#include "Utils.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

struct TraitEvent
{
    std::string trait;
    int attackModifier;
    std::string deed;
};

static const std::vector<TraitEvent> TRAIT_EVENTS = {
    {"Brave", 8, "holds the line steady"},
    {"Cowardly", -12, "hangs back and leaves a gap in the line"},
    {"Reckless", 12, "charges headfirst into the danger"},
};

static const int MAX_LUCK = 30;

static int teamPower(const Team &team, const Roster &roster) {
    int power = 0;
    for (int id : team.getMembersIds()) {
        const Stats s = roster.findUnitById(id).getStats();
        power += s.getStrength() + s.getConstitution();
    }
    return power;
}

void runIncursion(Team &team, Roster &roster, GameState &state, const std::vector<Encounter> &encounters, std::mt19937 &rng)
{
    if (team.getMembersIds().empty())
    {
        std::cout << "No units in the team. Assemble a team first." << std::endl;
        return;
    }

    state.incursionCount++;

    std::cout << std::endl;
    std::cout << "=== Incursion " << state.incursionCount << " ===" << std::endl;
    team.printTeam(roster);

    int startFloor = 1;
    if (state.highestFloor > 0)
    {
        std::cout << "Start from floor? [1-" << state.highestFloor + 1
                  << "] (toll: 1 essence per floor above the first)" << std::endl;
        startFloor = readChoice();
        if (startFloor < 1 || startFloor > state.highestFloor + 1)
            startFloor = 1;
    }

    int toll = startFloor - 1;
    if (toll > state.essence)
    {
        std::cout << "Not enough essence for floor " << startFloor << " (" << state.essence
                  << "/" << toll << "). The tower lets you in at floor 1." << std::endl;
        startFloor = 1;
        toll = 0;
    }
    if (toll > 0)
    {
        state.essence -= toll;
        std::cout << "The tower takes its toll: " << toll << " essence." << std::endl;
    }

    std::cout << std::endl;

    int highestFloorThisRun = 0;
    std::uniform_int_distribution<int> luck(0, MAX_LUCK);

    for (int floor = startFloor;; floor++)
    {
        int power = teamPower(team, roster);
        int danger = 20 + floor * 15;
        int attack = power + luck(rng);

        std::uniform_int_distribution<size_t> pickEnc(0, encounters.size() - 1);
        const Encounter &enc = encounters[pickEnc(rng)];
        std::cout << "Floor " << floor << ": " << enc.description << "." << std::endl;

        int forcedVictimId = -1;

        std::vector<std::pair<int, const TraitEvent *>> candidates;
        for (int id : team.getMembersIds())
        {
            const Unit &u = roster.findUnitById(id);
            for (const TraitEvent &ev : TRAIT_EVENTS)
                if (std::find(u.getSkills().begin(), u.getSkills().end(), ev.trait) != u.getSkills().end())
                    candidates.push_back({id, &ev});
        }

        if (!candidates.empty())
        {
            std::uniform_int_distribution<int> coin(0, 1);
            if (coin(rng) == 1)
            {
                auto selected = candidates[std::uniform_int_distribution<size_t>(0, candidates.size() - 1)(rng)];
                const TraitEvent *event = selected.second;

                const Unit &actor = roster.findUnitById(selected.first);
                std::cout << "  " << actor.getName() << ", " << event->trait << " as ever, "
                          << event->deed << "." << std::endl;

                attack += event->attackModifier;

                if (event->trait == "Reckless")
                    forcedVictimId = selected.first;
            }
        }

        if (attack >= danger * 12 / 10)
        {
            std::cout << "  The team advances with ease." << std::endl;
            state.essence += floor;
            std::cout << "  The floor yields " << floor << " essence." << std::endl;

            for (int id : team.getMembersIds())
                if (roster.findUnitById(id).addExperience(floor * 10) > 0)
                    std::cout << "  " << roster.findUnitById(id).getName() << " reaches level "
                              << roster.findUnitById(id).getLevel() << "!" << std::endl;

            highestFloorThisRun = floor;
            if (floor > state.highestFloor)
                state.highestFloor = floor;
        }
        else if (attack >= danger)
        {
            int victimId;
            if (forcedVictimId != -1)
                victimId = forcedVictimId;
            else
            {
                const std::vector<int> &ids = team.getMembersIds();
                std::uniform_int_distribution<size_t> pick(0, ids.size() - 1);
                victimId = ids[pick(rng)];
            }

            std::cout << "  The team advances with difficulty." << std::endl;
            state.essence += floor;
            std::cout << "  The floor yields " << floor << " essence." << std::endl;
            for (int id : team.getMembersIds())
                if (roster.findUnitById(id).addExperience(floor * 10) > 0)
                    std::cout << "  " << roster.findUnitById(id).getName() << " reaches level "
                              << roster.findUnitById(id).getLevel() << "!" << std::endl;

            std::uniform_int_distribution<int> dmg(15, 40);
            Unit &victim = roster.findUnitById(victimId);
            victim.takeDamage(dmg(rng));

            std::cout << "  " << victim.getName();
            if (!victim.getSkills().empty())
                std::cout << ", " << pickRandom(victim.getSkills(), rng) << " as ever, ";
            std::cout << "is wounded." << std::endl;

            if (!victim.isAlive())
            {
                std::cout << "  " << victim.getName();
                if (!victim.getSkills().empty())
                    std::cout << ", " << pickRandom(victim.getSkills(), rng) << " to the end,";
                std::cout << " falls on floor " << floor << ". The others mourn and press on." << std::endl;

                state.necropolis.addDeath(victim, floor, enc.cause, state.incursionCount);
                roster.removeUnitById(victimId);
                team.purgeDeadMembers(roster);

                if (team.getMembersIds().empty())
                {
                    std::cout << std::endl;
                    std::cout << "The tower claims them all. No one returns." << std::endl;
                    break;
                }
            }

            highestFloorThisRun = floor;
            if (floor > state.highestFloor)
                state.highestFloor = floor;
        }
        else
        {
            std::cout << "  The tower overwhelms the team." << std::endl;

            const std::vector<int> &ids = team.getMembersIds();
            std::uniform_int_distribution<size_t> pick(0, ids.size() - 1);
            int victimId = ids[pick(rng)];
            const Unit &fallen = roster.findUnitById(victimId);

            std::cout << "  " << fallen.getName();
            if (!fallen.getSkills().empty())
                std::cout << ", " << pickRandom(fallen.getSkills(), rng) << " to the end,";
            std::cout << " falls on floor " << floor << ". The others retreat." << std::endl;

            state.necropolis.addDeath(fallen, floor, enc.cause, state.incursionCount);
            roster.removeUnitById(victimId);
            team.purgeDeadMembers(roster);

            if (team.getMembersIds().empty())
            {
                std::cout << std::endl;
                std::cout << "The tower claims them all. No one returns." << std::endl;
            }

            break;
        }

        int nextDanger = 20 + (floor + 1) * 15;
        int currentPower = teamPower(team, roster);

        if (currentPower >= nextDanger * 12 / 10)
            std::cout << "The way up looks clear." << std::endl;
        else if (currentPower + MAX_LUCK >= nextDanger * 12 / 10)
            std::cout << "The air grows heavier." << std::endl;
        else if (currentPower + MAX_LUCK >= nextDanger)
            std::cout << "Something waits above, and it is not afraid of you." << std::endl;
        else 
            std::cout << "Climbing further is death." << std::endl;

        std::cout << "Climb to floor " << floor + 1 << "? [1] Yes  [2] Return" << std::endl;
        if (readChoice() != 1)
        {
            std::cout << "The team descends with their spoils and their lives." << std::endl;
            break;
        }
    }

    team.purgeDeadMembers(roster);

    for (int id : team.getMembersIds())
    {
        Unit &u = roster.findUnitById(id);
        u.heal(u.getStats().getMaxHealth());
    }

    std::cout << std::endl;
    std::cout << "=== Incursion " << state.incursionCount << " over ===" << std::endl;
    std::cout << "Highest floor this run: " << highestFloorThisRun
              << "  |  Tower record: " << state.highestFloor
              << "  |  Essence: " << state.essence << std::endl;
    if (!team.getMembersIds().empty())
    {
        std::cout << "The survivors rest and tend their wounds." << std::endl;
        team.printTeam(roster);
    }
}

std::vector<Encounter> loadEncounters(const std::string &path)
{
    std::vector<Encounter> encounters;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        size_t pos = line.find('|');
        if (pos == std::string::npos)
            throw std::runtime_error("Malformed line in " + path + ": " + line);

        std::string description = line.substr(0, pos);
        std::string cause = line.substr(pos + 1);

        encounters.push_back({description, cause});
    }
    if (encounters.empty())
        throw std::runtime_error("Encounter bank is empty or file not found: " + path);
    return encounters;
}
