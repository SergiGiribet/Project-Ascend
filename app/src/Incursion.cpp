#include "Incursion.h"
#include "Utils.h"

#include <iostream>

void runIncursion(Team &team, Roster &roster, GameState &state, std::mt19937 &rng)
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
        std::cout << "Start from floor? [1-" << state.highestFloor + 1 << "]" << std::endl;
        startFloor = readChoice();
        if (startFloor < 1 || startFloor > state.highestFloor + 1)
            startFloor = 1;
    }

    std::cout << std::endl;

    int highestFloorThisRun = 0;
    std::uniform_int_distribution<int> luck(0, 30);

    for (int floor = startFloor;; floor++)
    {
        int power = 0;
        for (int id : team.getMembersIds())
        {
            const Stats s = roster.findUnitById(id).getStats();
            power += s.getStrength() + s.getConstitution();
        }
        int danger = 20 + floor * 15;
        int attack = power + luck(rng);

        if (attack >= danger * 12 / 10)
        {
            std::cout << "Floor " << floor << ": The team advances with ease." << std::endl;
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
            std::cout << "Floor " << floor << ": The team advances with difficulty." << std::endl;
            for (int id : team.getMembersIds())
                if (roster.findUnitById(id).addExperience(floor * 10) > 0)
                    std::cout << "  " << roster.findUnitById(id).getName() << " reaches level "
                              << roster.findUnitById(id).getLevel() << "!" << std::endl;

            const std::vector<int> &ids = team.getMembersIds();
            std::uniform_int_distribution<size_t> pick(0, ids.size() - 1);
            int victimId = ids[pick(rng)];

            std::uniform_int_distribution<int> dmg(15, 40);
            Unit &victim = roster.findUnitById(victimId);
            victim.takeDamage(dmg(rng));

            std::cout << "  " << victim.getName();
            if (!victim.getSkills().empty())
                std::cout << ", " << pickRandom(victim.getSkills(), rng) << " as ever,";
            std::cout << " is wounded holding the line." << std::endl;

            if (!victim.isAlive())
            {
                std::cout << "  " << victim.getName();
                if (!victim.getSkills().empty())
                    std::cout << ", " << pickRandom(victim.getSkills(), rng) << " to the end,";
                std::cout << " falls on floor " << floor << ". The others mourn and press on." << std::endl;

                state.necropolis.addDeath(victim, floor, "wounded holding the line", state.incursionCount);
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
            std::cout << "Floor " << floor << ": The tower overwhelms the team." << std::endl;

            const std::vector<int> &ids = team.getMembersIds();
            std::uniform_int_distribution<size_t> pick(0, ids.size() - 1);
            int victimId = ids[pick(rng)];
            const Unit &fallen = roster.findUnitById(victimId);

            std::cout << "  " << fallen.getName();
            if (!fallen.getSkills().empty())
                std::cout << ", " << pickRandom(fallen.getSkills(), rng) << " to the end,";
            std::cout << " falls on floor " << floor << ". The others retreat." << std::endl;

            state.necropolis.addDeath(fallen, floor, "overwhelmed in the retreat", state.incursionCount);
            roster.removeUnitById(victimId);
            team.purgeDeadMembers(roster);

            if (team.getMembersIds().empty())
            {
                std::cout << std::endl;
                std::cout << "The tower claims them all. No one returns." << std::endl;
            }

            break;
        }

        std::cout << "Climb to floor " << floor + 1 << "? [1] Yes  [2] Return" << std::endl;
        if (readChoice() != 1)
        {
            std::cout << "The team descends with their spoils and their lives." << std::endl;
            break;
        }
    }

    team.purgeDeadMembers(roster);

    std::cout << std::endl;
    std::cout << "=== Incursion " << state.incursionCount << " over ===" << std::endl;
    std::cout << "Highest floor this run: " << highestFloorThisRun
              << "  |  Tower record: " << state.highestFloor << std::endl;
    if (!team.getMembersIds().empty())
        team.printTeam(roster);
}
