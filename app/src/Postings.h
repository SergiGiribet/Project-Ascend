// Postings is where every unit is stationed: in one party, in the training camp, or nowhere. It
// owns the Barracks and the TrainingCamp; nothing else does.
//
// IMPORTANT: it exists for the same reason Barracks does, one level up. "A unit is never in a
// party AND in the camp" is a rule about the space between the two, so neither can defend it
// alone: Barracks has never heard of the camp, and the camp has never heard of a party. Every
// path that puts a unit somewhere goes through here and frees it from the other side first; if
// that lived in the menus, it would be correct until the day one of them forgot -- which is
// exactly how main.cpp let a trainee walk into a party and die there unnoticed by the camp.
//
// The payoff: a unit in the camp can never be in the tower, so tickCamp can never reach anyone
// who is away, and a death in the tower only ever has to be purged from the party that climbed.
//
// Invariant: no unit id is both in a party and in the camp; Barracks keeps its own (one party at
//            most); every id refers to a living unit present in the roster.

#ifndef POSTINGS_H
#define POSTINGS_H

#include "Barracks.h"
#include "TrainingCamp.h"
#include "GameState.h"

#include <string>
#include <vector>
#include <random>

class Postings
{
public:
    // A unit a save listed in a party and in the camp at once, and what was done about it. Data
    // only: main composes the sentence (style guide §1, rules 1 and 3).
    struct Moved
    {
        enum class Kept { Camp, Party };

        int unitId;
        int partyIndex;                 // the party they were listed in
        Kept kept;                      // Camp: they left the party. Party: it was in the tower,
                                        // so they left the camp instead.
        std::vector<int> freedTrainees; // only when kept == Party and they were a trainer
    };

    // Builder ----------------------------------------------------------------------------------
    static Postings fromSave(Barracks barracks, TrainingCamp camp, const GameState &state,
                             std::vector<Moved> &moved);
    // Pre: barracks and camp as loadGame left them (each valid on its own; together they may
    //      break the invariant), and every sortie in state.sorties names a party that exists.
    // Post: The ONLY way to build a Postings. Repairs every unit found on both sides: it stays in
    //       the camp and leaves its party, unless that party is in the tower -- then the tower
    //       wins and it leaves the camp (a trainer takes its trainees' places with it: they are
    //       freed). Appends one Moved per repaired unit to `moved`, in party order. A new game is
    //       just an empty barracks and camp, with nothing to repair.

    // Consultors -------------------------------------------------------------------------------
    const Barracks &barracks() const;
    const TrainingCamp &camp() const;
    // Pre: None
    // Post: Read-only views, for printing, saving and asking who is where. Never non-const: a
    //       Barracks & or a TrainingCamp & would be a way round every rule below.

    Team &party(int partyIndex);
    // Pre: 0 <= partyIndex < barracks().count() (throws std::out_of_range otherwise).
    // Post: Returns that party, writable, for resolveSortie to wound and purge. Safe to hand out
    //       because Team::addMember is private to Barracks: through a Team & a unit can only
    //       leave. Same warning as Barracks::at: invalid after createParty() or disband().

    // Entrades: each frees the unit from wherever it was first ----------------------------------
    void assignToParty(int unitId, int partyIndex, const Roster &roster);
    // Pre: as Barracks::assign (throws if the party is full or the unit is not in the roster).
    //      The party must not be in the tower: the caller checks, as it does today.
    // Post: The unit is in that party and in no other place. If it was in the camp it left it;
    //       if it was a trainer, its trainees were freed with it.

    void addTrainer(int unitId, const Roster &roster);
    // Pre: as TrainingCamp::addTrainer (throws if already in the camp or no slot is free). The
    //      unit must not be in the tower.
    // Post: The unit is a trainer and in no party.

    void assignTrainee(int trainerId, int traineeId, const Roster &roster);
    // Pre: as TrainingCamp::assignTrainee. The trainee must not be in the tower.
    // Post: The trainee is under that trainer and in no party.

    // Sortides i neutres -----------------------------------------------------------------------
    int createParty(const std::string &name);
    // Post: as Barracks::create.

    void disband(int partyIndex);
    // Pre/Post: as Barracks::disband. Do not call it from the game: disbandParty (Incursion.h)
    //           does, and keeps Sortie::partyIndex in step.

    void removeFromParty(int partyIndex, int unitId);
    // Post: The unit is in no party; does nothing if it was not in that one.

    void removeTrainer(int trainerId);
    void removeTrainee(int traineeId);
    // Post: as the TrainingCamp methods of the same name.

    void buySlot();
    // Pre: the caller has already charged camp().nextSlotCost() essence.

    void tickCamp(Roster &roster, const std::vector<Injury> &injuries, std::mt19937 &rng);
    // Post: as TrainingCamp::tick. By the invariant, it never touches a unit in a party.

    // Morts ------------------------------------------------------------------------------------
    void purgeDead(const Roster &roster);
    // Pre: None
    // Post: Every id no longer in the roster is gone from every party and from the camp (a dead
    //       trainer's trainees are freed). Call it after any death outside a party's own sortie:
    //       today, the scout.

private:
    Postings(Barracks barracks, TrainingCamp camp);

    Barracks barracks_;
    TrainingCamp camp_;
};

#endif // POSTINGS_H
