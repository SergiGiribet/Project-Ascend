#include "src/Unit.h"
#include "src/Team.h"
#include "src/Necropolis.h"
#include "src/Roster.h"
#include "src/Generator.h"
#include "src/Incursion.h"
#include "src/Utils.h"
#include "src/GameState.h"
#include "src/Logger.h"
#include "src/Injury.h"

#include <iostream>
#include <ctime>

void printIntro()
{
    std::cout << std::endl;
    std::cout << "=== PROJECT ASCEND ===" << std::endl;
    std::cout << std::endl;
    std::cout << "The tower appeared without warning, and it has no top." << std::endl;
    std::cout << "Those who climb it do not come back. Yet the summoning" << std::endl;
    std::cout << "circle keeps offering souls -- each one with a name," << std::endl;
    std::cout << "a past, and a reason of their own to climb." << std::endl;
    std::cout << std::endl;
    std::cout << "Invoke them. Choose who ascends. Push higher, floor by" << std::endl;
    std::cout << "floor, and decide when courage becomes greed." << std::endl;
    std::cout << std::endl;
    std::cout << "When they die -- and they will -- they are gone for good." << std::endl;
    std::cout << "Only the Necropolis remembers. And sometimes, the newly" << std::endl;
    std::cout << "summoned arrive already carrying the echo of the fallen." << std::endl;
    std::cout << std::endl;
    std::cout << "How high will you climb on their shoulders?" << std::endl;
}

void Menu()
{
    std::cout << std::endl;
    std::cout << "=== PROJECT ASCEND ===" << std::endl;
    std::cout << "  1. Invoke a new unit" << std::endl;
    std::cout << "  2. Manage the team" << std::endl;
    std::cout << "  3. Enter the tower" << std::endl;
    std::cout << "  6. Visit the Necropolis" << std::endl;
    std::cout << "  8. View the roster" << std::endl;
    std::cout << "  9. Exit" << std::endl;
}

void TeamMenu()
{
    std::cout << std::endl;
    std::cout << "=== Team Management ===" << std::endl;
    std::cout << "  1. Add a unit to the team" << std::endl;
    std::cout << "  2. Remove a unit from the team" << std::endl;
    std::cout << "  3. View team composition" << std::endl;
    std::cout << "  6. Return to main menu" << std::endl;
}

void IncursionMenu()
{
    std::cout << std::endl;
    std::cout << "=== The Tower ===" << std::endl;
    std::cout << "  1. Start the incursion" << std::endl;
    std::cout << "  3. Return to main menu" << std::endl;
}

void printStatus(const GameState &state)
{
    std::cout << "Essence: " << COLOR_CYAN << state.essence << COLOR_RESET
              << "  |  Tower record: floor " << state.highestFloor << std::endl;
}

int main()
{
    enableConsoleColors();

    int choice = 0;
    try
    {

        std::time_t now = std::time(nullptr);
        std::tm tm;
        localtime_s(&tm, &now);
        char name[64];
        std::strftime(name, sizeof name, "sessions/session_%Y%m%d_%H%M%S.log", &tm);
        SessionLog log(name);

        Team team;
        Roster roster;
        GameState state;

        std::mt19937 rng(std::random_device{}());
        Generator gen("resources", rng);
        std::vector<Encounter> encounters = loadEncounters("resources/encounters.txt");
        std::vector<Injury> injuries = loadInjuries("resources/injuries.txt");
        int nextId = 1;

        printIntro();

        do
        {
            Menu();
            printStatus(state);

            choice = readChoice();

            switch (choice)
            {
            case 1:
            {
                std::cout << std::endl;
                if (state.essence >= state.invokeCost)
                {
                    state.essence -= state.invokeCost;
                    std::cout << "The circle drinks " << state.invokeCost << " essence." << std::endl;
                    std::cout << "The summoning circle glows..." << std::endl;
                    Unit newUnit = gen.generateUnit(nextId++, state.necropolis);
                    roster.addUnit(newUnit);
                    newUnit.printUnit();
                }
                else
                    std::cout << "Not enough essence to invoke: " << state.essence
                              << "/" << state.invokeCost << ". The circle stays dark." << std::endl;
                break;
            }
            case 2:
            {
                do
                {
                    TeamMenu();
                    choice = readChoice();

                    switch (choice)
                    {
                    case 1:
                    {
                        std::cout << std::endl;
                        std::cout << "Who joins the team? (unit id)" << std::endl;
                        roster.printRoster(team.getMembersIds());
                        team.printTeam(roster);
                        int selectedUnitId = readChoice();
                        try
                        {
                            team.addMember(selectedUnitId, roster);
                            std::cout << "Unit added to the team." << std::endl;
                        }
                        catch (const std::runtime_error &e)
                        {
                            std::cout << e.what() << std::endl;
                        }
                        break;
                    }
                    case 2:
                    {
                        std::cout << std::endl;
                        std::cout << "Who leaves the team? (unit id)" << std::endl;
                        team.printTeam(roster);
                        int selectedUnitId = readChoice();
                        team.removeMember(selectedUnitId);
                        team.printTeam(roster);
                        break;
                    }
                    case 3:
                    {
                        std::cout << std::endl;
                        team.printTeam(roster);
                        break;
                    }
                    case 6:
                    {
                        break;
                    }
                    default:
                    {
                        std::cout << "Invalid choice. Please try again." << std::endl;
                        break;
                    }
                    }

                } while (choice != 6);
                break;
            }
            case 3:
            {
                do
                {
                    IncursionMenu();
                    team.printTeam(roster);
                    printStatus(state);
                    choice = readChoice();

                    switch (choice)
                    {
                    case 1:
                    {
                        runIncursion(team, roster, state, encounters, injuries, rng);
                        break;
                    }
                    case 3:
                    {
                        break;
                    }
                    default:
                    {
                        std::cout << "Invalid choice. Please try again." << std::endl;
                        break;
                    }
                    }

                } while (choice != 3);
                break;
            }
            case 6:
            {
                std::cout << std::endl;
                state.necropolis.print();
                break;
            }
            case 8:
            {
                std::cout << std::endl;
                roster.printRoster(team.getMembersIds());
                printStatus(state);
                std::cout << "Incursions launched: " << state.incursionCount << std::endl;
                break;
            }
            case 9:
            {
                std::cout << "The tower will be waiting. Goodbye!" << std::endl;
                return 0;
            }
            default:
            {
                std::cout << "Invalid choice. Please try again." << std::endl;
                break;
            }
            }
        } while (choice != 9);
    }
    catch (const std::exception &e)
    {
        std::cout << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
