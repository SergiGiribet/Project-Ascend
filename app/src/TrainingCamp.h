#ifndef TRAININGCAMP_H
#define TRAININGCAMP_H

#include "Roster.h"
#include "Injury.h"

#include <vector>
#include <random>

class TrainingCamp
{
public:
    static const int TRAINEES_PER_TRAINER = 5;
    static const int STARTING_SLOTS = 1;

    std::vector<int> trainerIds() const;
    // Post: Returns the ids of all assigned trainers, in assignment order.

    std::vector<int> traineeIds() const;
    // Post: Returns the ids of all trainees across every trainer, in assignment order.

    TrainingCamp();
    // Pre: None.
    // Post: Creates a camp with STARTING_SLOTS trainer slots and no one assigned.

    // Consultors ---------------------------------------------------------------------------
    int purchasedSlots() const;
    // Pre: None.
    // Post: Returns how many trainer slots have been paid for (>= STARTING_SLOTS).

    int assignedTrainerCount() const;
    // Pre: None.
    // Post: Returns how many trainers are currently assigned (<= purchasedSlots()).

    int nextSlotCost() const;
    // Pre: None.
    // Post: Returns the essence cost of the next trainer slot (250 for the 2nd, then doubling).

    bool contains(int unitId) const;
    // Pre: None.
    // Post: Returns true if unitId is currently a trainer or a trainee in the camp.

    // Modifiers ----------------------------------------------------------------------------
    void buySlot();
    // Pre: the caller has already charged nextSlotCost() essence.
    // Post: Increases purchasedSlots() by one.

    void addTrainer(int unitId, const Roster &roster);
    // Pre: unitId is a living roster unit, not already in the camp, and
    //      assignedTrainerCount() < purchasedSlots() (throws std::runtime_error otherwise).
    // Post: Registers unitId as a trainer with no trainees yet.

    void assignTrainee(int trainerId, int traineeId, const Roster &roster);
    // Pre: trainerId is an assigned trainer; traineeId is a living roster unit not already in
    //      the camp; that trainer has < TRAINEES_PER_TRAINER trainees (throws otherwise).
    // Post: Adds traineeId under trainerId.

    void removeTrainer(int trainerId);
    // Post: Removes the trainer and releases all its trainees from the camp; does nothing if
    //       trainerId is not a trainer.

    void removeTrainee(int traineeId);
    // Post: Removes traineeId from whichever trainer holds it; does nothing if not found.

    void purgeDead(const Roster &roster);
    // Post: Drops any trainer or trainee no longer present in the roster (a trainer dropped
    //       this way also releases its surviving trainees).

    void tick(Roster &roster, const std::vector<Injury> &injuries, std::mt19937 &rng);
    // Post: Advances training by one incursion. Each trainee gains
    //       trainerLevel * 15 * (1 + (trainerRace - 1) * 0.15) XP (and may level up). Each
    //       trainee (2%) and trainer (0.3%) may suffer a training incident: 85% a normal wound,
    //       15% a permanent injury (applyInjury). Never fatal. Prints what happens.

    // Display ------------------------------------------------------------------------------
    void print(const Roster &roster) const;
    // Post: Lists trainers with their trainees (resolved through the roster), free slots, and
    //       the next slot's cost; prints a notice if the camp is empty.

private:
    struct Assignment
    {
        int trainerId;
        std::vector<int> traineeIds;
    };
    std::vector<Assignment> assignments_; // size <= purchasedSlots_
    int purchasedSlots_;
};

#endif