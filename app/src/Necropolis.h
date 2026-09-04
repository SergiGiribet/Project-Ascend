// Necropolis is the memory of the fallen. It does NOT store dead Unit objects: on permadeath the Unit is
// destroyed for real, and only its story remains here as a DeathRecord (name, where and how it died, when,
// and the traits it had). This is both the thematic point of the game (§3.2 of the GDD) and the mechanical
// heart of Fase 0: future invocations can be "marked" by a fallen one (the hook, step 6 of the plan).
//
// DeathRecord (lightweight struct):
//   - name        : who it was.
//   - floorDied   : where it fell.
//   - cause       : short text ("ambushed on floor 4").
//   - turn        : when, to keep records ordered.
//   - skills      : traits it had, to generate "marked" successors.
//
// Responsibilities:
//   - addDeath(const Unit &u, int floor, const std::string &cause, int turn): builds the record from the
//     unit JUST BEFORE the roster erases it (register first, then remove — order matters).
//   - pickRandom(rng): returns a random record, used by the generator to create hooks (step 6).
//   - print(): lists the fallen (for the "Visit Necropolis" menu).
//
// Invariant: records are append-only; the Necropolis never shrinks during a session.

#ifndef NECROPOLIS_H
#define NECROPOLIS_H

#include "Unit.h"

#include <string>
#include <vector>
#include <random>

struct DeathRecord {
    std::string name;
    int floorDied;
    std::string cause;
    int turn;
    std::vector<std::string> skills;
};

class Necropolis {
    public:
        Necropolis();
        // Pre: None
        // Post: Creates an empty necropolis.

        void addDeath(const Unit &unit, int floor, const std::string &cause, int turn);
        // Pre: unit must still exist (call BEFORE removing it from the roster).
        // Post: Appends a DeathRecord (name and traits taken from the unit, plus floor, cause
        //       and turn) to the registry.

        void addRecord(const DeathRecord &record);
        // Pre: None.
        // Post: Appends record as it is. This is for LOADING, not for dying: addDeath builds a record
        //       from a unit that is still on the roster, and on load there is no unit left to build it
        //       from. Keeping them apart is what stops a loaded game from having to invent one.

        bool empty() const;
        // Pre: None
        // Post: Returns true if no death has been recorded yet; false otherwise.

        const DeathRecord &pickRandom(std::mt19937 &rng) const;
        // Pre: the necropolis must not be empty (throws std::runtime_error otherwise).
        // Post: Returns a random death record (used by the generator for hooks).

        void print() const;
        // Pre: None
        // Post: Lists the fallen, oldest first; prints a notice if there are none.

        const std::vector<DeathRecord> &records() const;
        // Pre: None
        // Post: Returns a read-only view of every death, oldest first.

    private:
        std::vector<DeathRecord> records_;
};

#endif