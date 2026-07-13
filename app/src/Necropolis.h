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
