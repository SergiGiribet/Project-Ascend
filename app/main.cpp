#include "src/Unit.h"
#include "src/Team.h"
#include "src/Necropolis.h"
#include "src/Roster.h"
#include "src/Generator.h"
#include "src/Incursion.h"
#include "src/Utils.h"
#include "src/GameState.h"

#include <iostream>

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

int main()
{
    int choice = 0;
    try
    {
        Team team;
        Roster roster;
        GameState state;

        std::mt19937 rng(std::random_device{}());
        Generator gen("resources", rng);
        std::vector<Encounter> encounters = loadEncounters("resources/encounters.txt");
        int nextId = 1;

        do
        {
            Menu();

            choice = readChoice();

            switch (choice)
            {
            case 1:
            {
                std::cout << std::endl;
                std::cout << "The summoning circle glows..." << std::endl;
                Unit newUnit = gen.generateUnit(nextId++, state.necropolis);
                roster.addUnit(newUnit);
                newUnit.printUnit();
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
                        roster.printRoster();
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
                    std::cout << "Tower record: floor " << state.highestFloor << std::endl;
                    choice = readChoice();

                    switch (choice)
                    {
                    case 1:
                    {
                        runIncursion(team, roster, state, encounters, rng);
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
                roster.printRoster();
                std::cout << "Tower record: floor " << state.highestFloor
                          << "  |  Incursions launched: " << state.incursionCount << std::endl;
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
