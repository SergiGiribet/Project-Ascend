#include "src/Unit.h"
#include "src/Team.h"
#include "src/Necropolis.h"
#include "src/Roster.h"
#include "src/Generator.h"

#include <iostream>
#include <fstream>
#include <limits>

void Menu()
{

    std::cout << "Welcome to the Unit Management System!" << std::endl;
    std::cout << "1. Invoke a new unit" << std::endl;
    std::cout << "2. Modify team composition" << std::endl;
    std::cout << "3. Initialize a new incursion" << std::endl;
    std::cout << "6. Visit Necropolis" << std::endl;
    std::cout << "8. View stats" << std::endl;
    std::cout << "9. Exit" << std::endl;
    std::cout << "-------------------------" << std::endl;
}

void TeamMenu()
{
    std::cout << "Team Management Menu:" << std::endl;
    std::cout << "1. Add a unit to the team" << std::endl;
    std::cout << "2. Remove a unit from the team" << std::endl;
    std::cout << "3. View teams composition" << std::endl;
    std::cout << "6. Return to main menu" << std::endl;
    std::cout << "-------------------------" << std::endl;
}

void IncursionMenu()
{
    std::cout << "Incursion Menu:" << std::endl;
    std::cout << "Team 1: " << std::endl;
    // TODO: print the actual team once main owns the instances:
    //       IncursionMenu(const Team &team, const Roster &roster) -> team.printTeam(roster);
    std::cout << "1. Start the incursion" << std::endl;
    std::cout << "3. Return to main menu" << std::endl;
    std::cout << "-------------------------" << std::endl;
}

int readChoice()
{
    int c;
    if (!(std::cin >> c))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        c = 0;
    }
    return c;
}

int main()
{
    int choice = 0;
    try
    {
        Generator gen("resources");
        Roster roster;
        int nextId = 1;

        do
        {
            Menu();

            choice = readChoice();

            switch (choice)
            {
            case 1:
            {
                std::cout << "Invoking..." << std::endl;
                Unit newUnit = gen.generateUnit(nextId++);
                roster.addUnit(newUnit);
                std::cout << "Unit Invoked: " << std::endl;
                newUnit.printUnit();
                std::cout << std::endl;
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
                        std::cout << "Adding a unit to the team..." << std::endl;
                        // Add logic for adding a unit to the team
                        break;
                    }
                    case 2:
                    {
                        std::cout << "Removing a unit from the team..." << std::endl;
                        // Add logic for removing a unit from the team
                        break;
                    }
                    case 3:
                    {
                        std::cout << "Viewing team composition..." << std::endl;
                        // Add logic for viewing team composition
                        break;
                    }
                    case 6:
                    {
                        std::cout << "Returning to main menu..." << std::endl;
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
                    choice = readChoice();

                    switch (choice)
                    {
                    case 1:
                    {
                        std::cout << "Starting a new incursion..." << std::endl;
                        // Add logic for starting a new incursion
                        break;
                    }
                    case 3:
                    {
                        std::cout << "Returning to main menu..." << std::endl;
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
                std::cout << "Entering Necropolis..." << std::endl;
                // Add logic for visiting Necropolis
                break;
            }
            case 8:
            {
                std::cout << "Viewing stats..." << std::endl;
                // Add logic for viewing stats
                break;
            }
            case 9:
            {
                std::cout << "Exiting the program. Goodbye!" << std::endl;
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