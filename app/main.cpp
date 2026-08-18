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
#include "src/TrainingCamp.h"

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
    std::cout << "  3. Manage the training camp" << std::endl;
    std::cout << "  4. Enter the tower" << std::endl;
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

void TrainingCampMenu()
{
    std::cout << std::endl;
    std::cout << "=== Training Camp ===" << std::endl;
    std::cout << "  1. Assign a trainer" << std::endl;
    std::cout << "  2. Assign a trainee to a trainer" << std::endl;
    std::cout << "  3. Dismiss a trainer" << std::endl;
    std::cout << "  4. Dismiss a trainee" << std::endl;
    std::cout << "  5. Buy a trainer slot" << std::endl;
    std::cout << "  6. View the camp" << std::endl;
    std::cout << "  7. Return to menu" << std::endl;
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
        TrainingCamp tcamp;

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
                        roster.printRoster(team.getMembersIds(), tcamp.trainerIds(), tcamp.traineeIds());
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
                    TrainingCampMenu();
                    choice = readChoice();

                    switch (choice)
                    {
                    case 1:
                    {
                        std::cout << std::endl;
                        std::cout << "Who joins the camp as a trainer? (unit id)" << std::endl;
                        roster.printRoster(team.getMembersIds(), tcamp.trainerIds(), tcamp.traineeIds());
                        tcamp.print(roster);
                        int selectedUnitId = readChoice();
                        try
                        {
                            tcamp.addTrainer(selectedUnitId, roster);
                            team.removeMember(selectedUnitId);
                            std::cout << "Trainer assigned to the camp." << std::endl;
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
                        std::cout << "Who becomes a trainee? (unit id)" << std::endl;
                        roster.printRoster(team.getMembersIds(), tcamp.trainerIds(), tcamp.traineeIds());
                        tcamp.print(roster);
                        int selectedTraineeId = readChoice();
                        std::cout << "Under which trainer? (trainer id)" << std::endl;
                        int selectedTrainerId = readChoice();
                        try
                        {
                            tcamp.assignTrainee(selectedTrainerId, selectedTraineeId, roster);
                            team.removeMember(selectedTraineeId);
                            std::cout << "Trainee assigned to the camp." << std::endl;
                        }
                        catch (const std::runtime_error &e)
                        {
                            std::cout << e.what() << std::endl;
                        }
                        break;
                    }
                    case 3:
                    {
                        std::cout << std::endl;
                        std::cout << "Which trainer do you want to dismiss? (unit id)" << std::endl;
                        tcamp.print(roster);
                        int selectedUnitId = readChoice();
                        tcamp.removeTrainer(selectedUnitId);
                        break;
                    }
                    case 4:
                    {
                        std::cout << std::endl;
                        std::cout << "Which trainee do you want to dismiss? (unit id)" << std::endl;
                        tcamp.print(roster);
                        int selectedUnitId = readChoice();
                        tcamp.removeTrainee(selectedUnitId);
                        break;
                    }
                    case 5:
                    {
                        std::cout << std::endl;
                        std::cout << "Trainer slots owned: " << tcamp.purchasedSlots() << std::endl;
                        std::cout << "A new slot costs " << tcamp.nextSlotCost() << " essence. Buy it? [1] Yes  [2] No" << std::endl;
                        int selectedChoice = readChoice();
                        if (selectedChoice == 1)
                        {
                            if (state.essence >= tcamp.nextSlotCost())
                            {
                                state.essence -= tcamp.nextSlotCost();
                                tcamp.buySlot();
                                std::cout << "New trainer slot bought." << std::endl;
                            }
                            else
                                std::cout << "Not enough essence." << std::endl;
                        }
                        else if (selectedChoice != 2)
                            std::cout << "Invalid choice. Please try again." << std::endl;
                        break;
                    }
                    case 6:
                    {
                        std::cout << std::endl;
                        tcamp.print(roster);
                        break;
                    }
                    case 7:
                    {
                        break;
                    }

                    default:
                        std::cout << "Invalid choice. Please try again." << std::endl;
                        break;
                    }
                } while (choice != 7);
                break;
            }
            case 4:
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
                        if (!team.getMembersIds().empty())
                        {
                            runIncursion(team, roster, state, encounters, injuries, rng);
                            tcamp.tick(roster, injuries, rng);
                            roster.healRested(team.getMembersIds());
                        }
                        else
                            runIncursion(team, roster, state, encounters, injuries, rng); // imprimeix l'avís "No units"

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
                roster.printRoster(team.getMembersIds(), tcamp.trainerIds(), tcamp.traineeIds());
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
