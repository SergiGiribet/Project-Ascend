#include "TrainingCamp.h"

#include <iostream>
#include <algorithm>
#include <stdexcept>

TrainingCamp::TrainingCamp() : purchasedSlots_(STARTING_SLOTS) {}

std::vector<int> TrainingCamp::trainerIds() const
{
    std::vector<int> ids;
    for (const Assignment &a : assignments_)
        ids.push_back(a.trainerId);
    return ids;
}

std::vector<int> TrainingCamp::traineeIds() const
{
    std::vector<int> ids;
    for (const Assignment &a : assignments_)
        for (int id : a.traineeIds)
            ids.push_back(id);
    return ids;
}

int TrainingCamp::purchasedSlots() const
{
    return purchasedSlots_;
}

int TrainingCamp::assignedTrainerCount() const
{
    return static_cast<int>(assignments_.size());
}

int TrainingCamp::nextSlotCost() const
{
    return 250 * (1 << (purchasedSlots_ - 1));
}

bool TrainingCamp::contains(int unitId) const
{
    for (const Assignment &a : assignments_)
    {
        if (a.trainerId == unitId)
            return true;
        if (std::find(a.traineeIds.begin(), a.traineeIds.end(), unitId) != a.traineeIds.end())
            return true;
    }
    return false;
}

const std::vector<TrainingCamp::Assignment> &TrainingCamp::assignments() const
{
    return assignments_;
}

void TrainingCamp::buySlot()
{
    purchasedSlots_++;
}

void TrainingCamp::addTrainer(int unitId, const Roster &roster)
{
    if (!contains(unitId))
    {
        if (roster.contains(unitId))
        {
            if (assignedTrainerCount() >= purchasedSlots())
                throw std::runtime_error("No free trainer slots. Buy one first.");

            Assignment a;
            a.trainerId = unitId;
            assignments_.push_back(a);
        }
        else
        {
            throw std::runtime_error("No unit with that id exists in the roster.");
        }
    }
    else
    {
        throw std::runtime_error("That unit is already in the training camp.");
    }
}

void TrainingCamp::assignTrainee(int trainerId, int traineeId, const Roster &roster)
{
    if (contains(traineeId))
        throw std::runtime_error("That unit is already in the training camp.");
    if (!roster.contains(traineeId))
        throw std::runtime_error("No unit with that id exists in the roster.");
    for (Assignment &a : assignments_)
    {
        if (a.trainerId == trainerId)
        {
            if (a.traineeIds.size() >= TRAINEES_PER_TRAINER)
                throw std::runtime_error("That trainer already has a full roster of trainees.");
            a.traineeIds.push_back(traineeId);
            return;
        }
    }
    throw std::runtime_error("That trainer is not assigned to the camp.");
}

void TrainingCamp::removeTrainer(int trainerId)
{
    if (TrainingCamp::contains(trainerId))
    {
        assignments_.erase(
            std::remove_if(assignments_.begin(), assignments_.end(),
                           [trainerId](const Assignment &a)
                           { return a.trainerId == trainerId; }),
            assignments_.end());
    }
}

void TrainingCamp::removeTrainee(int traineeId)
{
    for (Assignment &a : assignments_)
    {
        a.traineeIds.erase(
            std::remove(a.traineeIds.begin(), a.traineeIds.end(), traineeId),
            a.traineeIds.end());
    }
}

void TrainingCamp::purgeDead(const Roster &roster)
{
    assignments_.erase(
        std::remove_if(assignments_.begin(), assignments_.end(),
                       [&roster](const Assignment &a)
                       { return !roster.contains(a.trainerId); }),
        assignments_.end());
    for (Assignment &a : assignments_)
        a.traineeIds.erase(
            std::remove_if(a.traineeIds.begin(), a.traineeIds.end(),
                           [&roster](int id)
                           { return !roster.contains(id); }),
            a.traineeIds.end());
}

// A training mishap: 15% a permanent injury, 85% a passing scrape that never kills (HP floored
// at 1). Shared by trainees and the (much safer) trainer.
static void applyTrainingHarm(Unit &u, const std::vector<Injury> &injuries, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> severity(1, 100);
    if (severity(rng) <= 15)
    {
        std::string injury = applyInjury(u, injuries, rng);
        std::cout << "  " << u.getName() << " is maimed in a training accident -- now "
                  << injury << "." << std::endl;
    }
    else
    {
        Stats s = u.getStats();
        std::uniform_int_distribution<int> dmg(10, 25);
        s.setHealth(std::max(1, s.getHealth() - dmg(rng)));
        u.setStats(s);
        std::cout << "  " << u.getName() << " takes a hard knock at the drills, but shakes it off."
                  << std::endl;
    }
}

void TrainingCamp::tick(Roster &roster, const std::vector<Injury> &injuries, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> pct(1, 100);       // trainee incident: 2%
    std::uniform_int_distribution<int> permille(1, 1000); // trainer incident: 0.3%

    for (Assignment &a : assignments_)
    {
        const Unit &trainer = roster.findUnitById(a.trainerId);
        int xp = static_cast<int>(trainer.getLevel() * 15 * (1.0 + (trainer.getRace() - 1) * 0.15));

        for (int id : a.traineeIds)
        {
            Unit &t = roster.findUnitById(id);
            if (t.addExperience(xp) > 0)
                std::cout << "  " << t.getName() << " reaches level " << t.getLevel()
                          << " under " << trainer.getName() << "'s drills." << std::endl;
            if (pct(rng) <= 2)
                applyTrainingHarm(t, injuries, rng);
        }

        Unit &tr = roster.findUnitById(a.trainerId);
        if (permille(rng) <= 3)
            applyTrainingHarm(tr, injuries, rng);
    }
}

void TrainingCamp::print(const Roster &roster) const
{
    if (assignments_.empty())
        std::cout << "Training camp is empty" << std::endl;
    else
        for (const Assignment &a : assignments_)
        {
            const Unit &trainer = roster.findUnitById(a.trainerId);
            const Stats s = trainer.getStats();
            std::cout << "  [" << trainer.getId() << "] " << trainer.getName()
                      << " (" << trainer.getRace() << "*)"
                      << " - Lv " << trainer.getLevel()
                      << " - HP " << s.getHealth() << "/" << s.getMaxHealth()
                      << " - XP " << trainer.getExperience() << std::endl;
            for (int id : a.traineeIds)
            {
                const Unit &t = roster.findUnitById(id);
                const Stats st = t.getStats();
                std::cout << "      [" << t.getId() << "] " << t.getName()
                          << " (" << t.getRace() << "*)"
                          << " - Lv " << t.getLevel()
                          << " - HP " << st.getHealth() << "/" << st.getMaxHealth()
                          << " - XP " << t.getExperience() << std::endl;
            }
        }
    std::cout << "Free trainer slots: " << (purchasedSlots() - assignedTrainerCount())
              << " of " << purchasedSlots()
              << "  |  Next slot: " << nextSlotCost() << " essence" << std::endl;
}