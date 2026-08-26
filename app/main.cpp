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
#include "src/Barracks.h"

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
    std::cout << "  2. Manage the parties" << std::endl;
    std::cout << "  3. Manage the training camp" << std::endl;
    std::cout << "  4. Enter the tower" << std::endl;
    std::cout << "  6. Visit the Necropolis" << std::endl;
    std::cout << "  8. View the roster" << std::endl;
    std::cout << "  9. Exit" << std::endl;
}

void TeamMenu()
{
    std::cout << std::endl;
    std::cout << "=== The Parties ===" << std::endl;
    std::cout << "  1. Assign a unit to a party" << std::endl;
    std::cout << "  2. Remove a unit from a party" << std::endl;
    std::cout << "  3. View the parties" << std::endl;
    std::cout << "  4. Form a new party" << std::endl;
    std::cout << "  5. Disband a party" << std::endl;
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
    std::cout << "  2. Send a scout to the next floor" << std::endl;
    std::cout << "  3. Return to main menu" << std::endl;
}

void ViewMenu()
{
    std::cout << std::endl;
    std::cout << "=== View Roster ===" << std::endl;
    std::cout << "  1. View Unit Details" << std::endl;
    std::cout << "  8. Return to main menu" << std::endl;
}

// Asks which party to act on. Returns its index, or -1 if the player backs out or names
// one that does not exist -- callers print nothing extra and simply do nothing.
int pickParty(Barracks &barracks, const Roster &roster)
{
    if (barracks.count() == 0)
    {
        std::cout << "No parties formed yet." << std::endl;
        return -1;
    }
    barracks.printAll(roster);
    std::cout << "Which party? (number, 0 to cancel)" << std::endl;
    int number = readChoice();
    if (number < 1 || number > barracks.count())
        return -1;
    return number - 1; // the player counts from one, the vector from zero
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

        Barracks barracks;
        Roster roster;
        GameState state;
        TrainingCamp tcamp;

        barracks.create("Party 1");

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
                int affordable = state.essence / state.invokeCost;
                if (affordable == 0)
                {
                    std::cout << "Not enough essence to invoke: " << state.essence
                              << "/" << state.invokeCost << ". The circle stays dark." << std::endl;
                    break;
                }

                std::cout << "How many? (up to " << affordable << ", 0 to cancel)" << std::endl;
                int count = readChoice();
                if (count <= 0)
                    break;
                if (count > affordable)
                {
                    std::cout << "There is only essence enough for " << affordable << "." << std::endl;
                    count = affordable;
                }

                state.essence -= count * state.invokeCost;
                std::cout << "The circle drinks " << count * state.invokeCost << " essence." << std::endl;
                std::cout << "The summoning circle glows..." << std::endl;

                for (int i = 0; i < count; ++i)
                {
                    Unit newUnit = gen.generateUnit(nextId++, state.necropolis);
                    roster.addUnit(newUnit);
                    newUnit.printUnit();
                }
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
                        int partyIndex = pickParty(barracks, roster);
                        if (partyIndex < 0)
                            break;
                        std::cout << "Who joins " << barracks.at(partyIndex).getName() << "? (unit id)" << std::endl;
                        roster.printRoster(barracks.memberTags(), tcamp.trainerIds(), tcamp.traineeIds());
                        int selectedUnitId = readChoice();
                        try
                        {
                            bool already = barracks.teamOfUnit(selectedUnitId) == partyIndex;
                            barracks.assign(selectedUnitId, partyIndex, roster);
                            std::cout << roster.findUnitById(selectedUnitId).getName()
                                      << (already ? " already answers to " : " joins ")
                                      << barracks.at(partyIndex).getName() << "." << std::endl;
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
                        int partyIndex = pickParty(barracks, roster);
                        if (partyIndex < 0)
                            break;
                        std::cout << "Who leaves " << barracks.at(partyIndex).getName() << "? (unit id)" << std::endl;
                        barracks.at(partyIndex).printTeam(roster);
                        int selectedUnitId = readChoice();
                        barracks.at(partyIndex).removeMember(selectedUnitId);
                        barracks.at(partyIndex).printTeam(roster);
                        break;
                    }
                    case 3:
                    {
                        std::cout << std::endl;
                        barracks.printAll(roster);
                        break;
                    }
                    case 4:
                    {
                        std::cout << std::endl;
                        std::string partyName = "Party " + std::to_string(barracks.count() + 1);
                        barracks.create(partyName);
                        std::cout << partyName << " forms up, empty and waiting." << std::endl;
                        break;
                    }
                    case 5:
                    {
                        std::cout << std::endl;
                        if (barracks.count() <= 1)
                        {
                            std::cout << "The last party cannot be disbanded." << std::endl;
                            break;
                        }
                        int partyIndex = pickParty(barracks, roster);
                        if (partyIndex < 0)
                            break;
                        std::cout << barracks.at(partyIndex).getName() << " is no more; its members answer to no one." << std::endl;
                        barracks.disband(partyIndex);
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
                        roster.printRoster(barracks.memberTags(), tcamp.trainerIds(), tcamp.traineeIds());
                        tcamp.print(roster);
                        int selectedUnitId = readChoice();
                        try
                        {
                            tcamp.addTrainer(selectedUnitId, roster);
                            barracks.release(selectedUnitId);
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
                        roster.printRoster(barracks.memberTags(), tcamp.trainerIds(), tcamp.traineeIds());
                        tcamp.print(roster);
                        std::cout << "Under which trainer? (trainer id)" << std::endl;
                        int trainerId = readChoice();

                        int traineeId = 0;
                        do {
                            std::cout << "Who becomes a trainee? (unit id, 0 to stop)" << std::endl;
                            traineeId = readChoice();
                            if (traineeId == 0)
                                break; 
                            try
                            {
                                tcamp.assignTrainee(trainerId, traineeId, roster);
                                barracks.release(traineeId);
                                std::cout << "Trainee assigned to the camp." << std::endl;
                            }
                            catch (const std::runtime_error &e)
                            {
                                std::cout << e.what() << std::endl;
                            }
                        } while (true);
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
                    printStatus(state);
                    choice = readChoice();

                    switch (choice)
                    {
                    case 1:
                    {
                        int partyIndex = pickParty(barracks, roster);
                        if (partyIndex < 0)
                            break;
                        // The ids are copied out BEFORE the incursion: the party loses its dead
                        // during it, and whoever climbed must not rest even if they fell.
                        std::vector<int> climbed = barracks.at(partyIndex).getMembersIds();
                        runIncursion(barracks.at(partyIndex), roster, state, encounters, injuries, rng);
                        if (!climbed.empty())
                        {
                            tcamp.tick(roster, injuries, rng);
                            roster.healRested(climbed);
                        }
                        break;
                    }
                    case 2:
                    {
                        std::cout << std::endl;
                        std::cout << "Who goes up to look? (unit id)" << std::endl;
                        roster.printRoster(barracks.memberTags(), tcamp.trainerIds(), tcamp.traineeIds());
                        int scoutId = readChoice();
                        int owner = barracks.teamOfUnit(scoutId);
                        runScoutMission(scoutId, barracks.at(owner >= 0 ? owner : 0), roster, state, rng);
                        barracks.purgeDead(roster); // the scout may belong to any party, or to none
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
                roster.printRoster(barracks.memberTags(), tcamp.trainerIds(), tcamp.traineeIds());
                ViewMenu();
                choice = readChoice();
                switch (choice)
                {
                case 1:
                {
                    int inspectedId = 0;
                    do {
                        std::cout << "Inspect a unit? (unit id, or 0 to return)" << std::endl;
                        inspectedId = readChoice();
                        if (inspectedId == 0)
                            break;
                        if (roster.contains(inspectedId))
                            roster.findUnitById(inspectedId).printUnit();
                        else
                            std::cout << "No one by that id is on the roster." << std::endl;
                    } while (true);
                    break;
                }
                case 8:
                {
                    break;
                }
                }
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
