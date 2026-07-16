#include "Incursion.h"
#include "Utils.h"

void runIncursion(Team &team, Roster &roster, GameState &state, std::mt19937 &rng)
{

    if (team.getMembersIds().empty())
    {
        std::cout << "No units in the team. Assemble a team first." << std::endl;
        return;
    }

    state.incursionCount++;

    std::cout << "Incursion Started" << std::endl;
    team.printTeam(roster);
    std::cout << "Tower record: floor " << state.highestFloor << std::endl;

    std::uniform_int_distribution<int> luck(0, 30);

    int startFloor = 1;
    int highestFloorThisRun = 0;
    if (state.highestFloor > 0)
    {
        std::cout << "Start from floor? (1-" << state.highestFloor + 1 << ")" << std::endl;
        startFloor = readChoice();
        if (startFloor < 1 || startFloor > state.highestFloor + 1)
            startFloor;
    }

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
            std::cout << "Floor " << floor << ": the team advances with ease." << std::endl;
            for (int id : team.getMembersIds())
                roster.findUnitById(id).addExperience(floor * 10);
            state.highestFloor = floor;
            highestFloorThisRun = floor;

            std::cout << "Floor " << floor << " cleared. Continue climbing? [1] Yes [2] Return" << std::endl;
            if (readChoice() != 1)
            {
                std::cout << "The team descends with their spoils and their lives." << std::endl;
                break;
            }
        }
        else if (attack >= danger)
        {
            std::cout << "Floor " << floor << ": the team advances with difficulty." << std::endl;
            for (int id : team.getMembersIds())
            {
                if (roster.findUnitById(id).addExperience(floor * 10) > 0)
                    std::cout << roster.findUnitById(id).getName() << " reaches level "
                              << roster.findUnitById(id).getLevel() << "!" << std::endl;
            }

            const std::vector<int> &ids = team.getMembersIds();
            std::uniform_int_distribution<size_t> pick(0, ids.size() - 1);
            int victimId = ids[pick(rng)];

            std::uniform_int_distribution<int> dmg(15, 40);
            Unit &victim = roster.findUnitById(victimId);
            victim.takeDamage(dmg(rng));
            std::cout << victim.getName() << " is wounded holding the line on floor " << floor << "." << std::endl;
            if (!victim.isAlive())
            {
                std::string fallenName = roster.findUnitById(victimId).getName();
                std::cout << fallenName << " falls on floor " << floor << ". The others cry the loss and continue with the mission." << std::endl;

                state.necropolis.addDeath(victim, floor, "wounded holding the line", state.incursionCount);

                roster.removeUnitById(victimId);
                team.purgeDeadMembers(roster);

                if (team.getMembersIds().empty())
                {
                    std::cout << "The tower claims them all. No one returns." << std::endl;
                    break;
                }
            }
            state.highestFloor = floor;
            highestFloorThisRun = floor;

            std::cout << "Floor " << floor << " cleared. Continue climbing? [1] Yes [2] Return" << std::endl;
            if (readChoice() != 1)
            {
                std::cout << "The team descends with their spoils and their lives." << std::endl;
                break;
            }
        }
        else
        {
            const std::vector<int> &ids = team.getMembersIds();
            std::uniform_int_distribution<size_t> pick(0, ids.size() - 1);
            int victimId = ids[pick(rng)];
            std::string fallenName = roster.findUnitById(victimId).getName();
            std::cout << fallenName << " falls on floor " << floor << ". The others retreat." << std::endl;

            state.necropolis.addDeath(roster.findUnitById(victimId), floor, "overwhelmed in the retreat", state.incursionCount);

            roster.removeUnitById(victimId);
            team.purgeDeadMembers(roster);
            if (team.getMembersIds().empty())
            {
                std::cout << "The tower claims them all. No one returns." << std::endl;
                break;
            }

            break;
        }
    }

    team.purgeDeadMembers(roster);
    std::cout << "The incursion is over. Highest floor this run: " << highestFloorThisRun
              << ". Tower record: " << state.highestFloor << std::endl;
}