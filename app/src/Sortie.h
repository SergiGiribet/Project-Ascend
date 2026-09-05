#ifndef SORTIE_H
#define SORTIE_H

// A party that is inside the tower right now.
//
// What is stored here is only what cannot be worked out again: which party went, where, when they
// left, and the seed. Everything else -- how long the sortie takes, when they are due, who dies --
// is REPLAYED from these four whenever anyone looks, on copies of the units, and committed to the
// roster only once the sortie is over. That is what makes the save unreadable as a spoiler: the
// outcome is not in it, and cannot be, because it does not exist anywhere until it is simulated.
struct Sortie {
    int partyIndex = -1;            // Index into Barracks; a party has at most one sortie at a time.
    int floor = 0;                  // Which floor they went to.
    long long departedAt = 0;       // nowSeconds() at the moment they left. Real time, not game time.
    unsigned int seed = 0;          // Seeds a std::mt19937 of the sortie's own, so the replay is
                                    // reproducible and does NOT depend on what the player did at home
                                    // while they were away.
};

#endif