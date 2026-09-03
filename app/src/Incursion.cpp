#include "Incursion.h"
#include "Utils.h"
#include "Objective.h"
#include "Scouting.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

struct TraitEvent
{
    std::string trait;
    int attackModifier;
    std::vector<std::string> deeds;
};

static const std::vector<TraitEvent> DAMAGE_TRAIT_EVENTS = {
    {"Brave", 8, {
                     "holds the line steady",
                     "steadies the wavering flank",
                     "meets the charge without flinching",
                     "digs in and holds fast",
                     "rallies the others with a steady shout",
                 }},
    {"Cowardly", -12, {
                          "hangs back and leaves a gap in the line",
                          "flinches at the first blow and gives ground",
                          "keeps one eye on the stairs down",
                          "lets the formation buckle",
                          "shrinks from the danger and drags the pace",
                      }},
    {"Reckless", 12, {
                         "charges headfirst into the danger",
                         "throws caution aside and presses the attack",
                         "dives into the thick of it",
                         "breaks ranks to strike the first blow",
                         "swings wild, heedless of the risk",
                     }},
};

// Round-by-round narration for Hold objectives. No trailing period: the line composes it.
static const std::vector<std::string> HOLD_HELD = {
    "the line holds",
    "they give nothing",
    "whatever comes, it breaks on them",
    "they hold, shoulder to shoulder",
    "the wall does not move",
};

static const std::vector<std::string> HOLD_GAVE_GROUND = {
    "the line buckles",
    "they are forced back a step",
    "something gets through",
    "the formation opens, and the floor finds the gap",
    "they give ground, and pay for it",
};

// The exchange with whatever holds the floor, chosen by how far the roll beat or missed the
// floor's difficulty. No trailing period: the line composes it.
static const std::vector<std::string> SLAY_CRUSHED = {
    "The blow goes in to the hilt",
    "They break something that will not mend",
    "It reels, and they do not let it recover",
    "The whole line lands at once",
    "It did not see that coming",
};

static const std::vector<std::string> SLAY_LANDED = {
    "They find the smallest opening",
    "A blow lands true",
    "It staggers",
    "The line presses in and something gives",
    "They open a wound that will not close",
};

static const std::vector<std::string> SLAY_BLOCKED = {
    "It turns the blow aside at the last moment",
    "The strike lands on nothing that matters",
    "It takes the hit on its guard and gives ground",
    "They almost had it",
    "The opening closes before they reach it",
};

static const std::vector<std::string> SLAY_MISSED = {
    "The strike never had a chance",
    "It is not where they swing",
    "They hit air, and it makes them pay",
    "The blow is nowhere near",
    "It lets them tire themselves out",
};

// Retrieve: a pass that turns nothing up. No trailing period; the line composes it.
static const std::vector<std::string> RETRIEVE_NOTHING = {
    "Nothing. Dust, and the sound of their own breathing",
    "They turn over what there is to turn over, and it is nothing",
    "Whatever this floor is keeping, it is not keeping it here",
    "Nothing yet. Somewhere above them, something shifts",
    "They work the far wall and come back with their hands empty",
};

// Retrieve: the pass that finds it.
static const std::vector<std::string> RETRIEVE_FOUND = {
    "It is under a fallen beam, and it takes three of them to lift it",
    "It had been walled in. The wall does not hold",
    "They find it where nothing should have been left",
    "It is smaller than they expected, and much heavier",
    "It comes away from the floor with a sound none of them like",
};

// Retrieve: a pass made after they already had what they came for.
static const std::vector<std::string> RETRIEVE_MORE = {
    "There is more, and they take it",
    "Another cache, tucked behind the first",
    "The floor was keeping two things. Now it keeps neither",
    "They lever up the next slab and are paid for it",
};

// Rescue: a round that closes on the captive. No trailing period; the line composes it.
static const std::vector<std::string> RESCUE_CLOSER = {
    "A door gives way ahead of them",
    "The way opens, and they take it",
    "One more corridor falls behind them",
    "They cut through and gain ground",
    "They are closer now, and something knows it",
};

// Rescue: a round that goes nowhere. The floor is not fighting them, it is delaying them.
static const std::vector<std::string> RESCUE_HELD = {
    "The floor closes ranks in front of them",
    "They lose the way and have to double back",
    "Something stands where the way was",
    "They are turned around, and it costs",
};

// Retrieve: the floor working out that they are still here. This is the warning.
static const std::vector<std::string> RETRIEVE_NOTICED = {
    "The floor has noticed them",
    "Something knows they are still here",
    "The quiet changes shape",
    "They are not alone in the room any more",
};

struct IncidentFlavor
{
    std::string cause;     // lowercase, no trailing period (necropolis composes the sentence)
    std::string woundLine; // full clause printed when the incident is not fatal
};

static const std::vector<IncidentFlavor> BOASTER_INCIDENTS = {
    {"impaled by a stray lance mid-boast", "trips over their own war story and catches a lance graze"},
    {"silenced mid-tale by a blade they never saw", "bites through their tongue mid-brag as a blade nicks past"},
    {"undone by their own legend, one line before the end", "stumbles mid-boast and takes a nasty spill"},
    {"buried under the weight of their own tall tale", "trips over their own swagger and goes down hard"},
};

static const std::vector<IncidentFlavor> NEUTRAL_INCIDENTS = {
    {"lost to a stray arrow no one saw coming", "is grazed by a stray arrow from nowhere"},
    {"claimed by a crack in the floor that wasn't there before", "stumbles into a crack that wasn't there a moment ago"},
    {"taken by an accident too strange to explain", "is caught by something too strange to explain"},
    {"crushed beneath a stone that had held for a thousand years, until now", "is clipped by a falling stone that had held for a thousand years, until now"},
};

// How far a roll can stray from what the party is worth, as a fraction of the floor's own
// difficulty. Flat luck made the game more deterministic the deeper it went -- a fixed 30 against
// a difficulty that climbs 15 a floor -- and being one-sided it meant power >= difficulty could
// never lose. A share of the floor keeps the uncertain band the same width at every depth, and
// making it symmetric puts that band around parity instead of entirely below it.
static const int LUCK_DIVISOR = 6;

static int luckSpan(int danger)
{
    return std::max(1, danger / LUCK_DIVISOR);
}
static const int BASE_INCIDENT_CHANCE = 3;
static const double DEPTH_INCREMENT = 0.3;
static const int INCIDENT_CAP = 18;
static const int FATAL_CHANCE = 18;
static const int WOUND_INJURY_CHANCE = 8;
static const int VICTIM_WEIGHT_BONUS = 2;
static const int VICTIM_WEIGHT_PENALTY = 1;
static const int SCOUT_RISK_BASE = 5;
static const int SCOUT_RISK_DIVISOR = 6;
static const int SCOUT_RISK_TRAIT = 10;
static const int SCOUT_RISK_MIN = 5;
static const int SCOUT_RISK_MAX = 60;
static const int SCOUT_WOUND_MIN = 10;
static const int SCOUT_WOUND_MAX = 25;
static const int WOUND_DMG_MIN = 15;
static const int WOUND_DMG_MAX = 40;
static const int HOLD_DMG_MIN = 10;
static const int HOLD_DMG_MAX = 25;
static const int SLAY_COUNTERS_ENDURED = 3;
static const int SLAY_CRIT_CHANCE = 8;
static const int SLAY_DMG_MIN = 15;
static const int SLAY_DMG_MAX = 40;
static const int EXPOSURE_STEP = 12;
static const int PUSH_BASE_SEARCHING = 85;
static const int PUSH_BASE_FOUND = 25;
static const int PUSH_GREEDY = 25;
static const int PUSH_CURIOUS = 15;
static const int PUSH_COWARDLY = 30;
static const int PUSH_FATIGUE = 10;
static const int EXPOSURE_CAUGHT = 100;

static int teamPower(const Team &team, const Roster &roster)
{
    int power = 0;
    for (int id : team.getMembersIds())
    {
        const Stats s = roster.findUnitById(id).getStats();
        power += s.getStrength() + s.getConstitution();
    }
    return power;
}

static int teamFit(const Team &team, const Roster &roster, ObjectiveType type)
{
    int fit = 0;
    for (int id : team.getMembersIds())
    {
        fit += traitFit(type, roster.findUnitById(id).getSkills());
    }
    return fit;
}

static int floorRoll(const Team &team, const Roster &roster, ObjectiveType type,
                     int traitMod, int danger, std::mt19937 &rng)
{
    const int span = luckSpan(danger);
    std::uniform_int_distribution<int> luck(-span, span);
    return teamPower(team, roster) + teamFit(team, roster, type) + traitMod + luck(rng);
}

static Objective objectiveFor(int floor, GameState &state, std::mt19937 &rng)
{
    auto it = state.floorObjectives.find(floor);
    if (it == state.floorObjectives.end())
        it = state.floorObjectives.emplace(floor, makeObjective(floor, rng)).first;
    return it->second;
}

static void awardFloor(int floor, const Team &team, Roster &roster, GameState &state)
{
    state.essence += floor;
    std::cout << "  The floor yields " << floor << " essence." << std::endl;
    for (int id : team.getMembersIds())
        if (roster.findUnitById(id).addExperience(floor * 10) > 0)
            std::cout << "  " << roster.findUnitById(id).getName() << " reaches level "
                      << roster.findUnitById(id).getLevel() << "!" << std::endl;
}

static void printForecast(int power, int danger)
{
    const int span = luckSpan(danger);
    if (power - span >= danger)
        std::cout << COLOR_GREEN << "The way up looks clear." << COLOR_RESET << std::endl;
    else if (power >= danger)
        std::cout << COLOR_YELLOW << "The air grows heavier." << COLOR_RESET << std::endl;
    else if (power + span >= danger)
        std::cout << COLOR_RED << "Something waits above, and it is not afraid of you." << COLOR_RESET << std::endl;
    else
        std::cout << COLOR_RED << "Climbing further is death." << COLOR_RESET << std::endl;
}

static std::string describeSurvivor(int enemyHp, int enemyHpStart)
{
    if (enemyHp * 3 > enemyHpStart * 2)
        return "It is barely marked.";
    if (enemyHp * 3 > enemyHpStart)
        return "It bleeds, but it is still standing.";
    return "One more blow might have done it.";
}

static bool hasTrait(const std::vector<std::string> &traits, const std::string &trait)
{
    return std::find(traits.begin(), traits.end(), trait) != traits.end();
}

static int riskDirection(const std::vector<std::string> &skills)
{
    if (hasTrait(skills, "Reckless") || hasTrait(skills, "Boaster"))
        return 1;
    if (hasTrait(skills, "Alert"))
        return -1;
    return 0;
}

static int scoutRisk(const Unit &scout, const Objective &objective)
{
    const Stats s = scout.getStats();
    int power = s.getStrength() + s.getConstitution();
    int risk = SCOUT_RISK_BASE + (objective.difficulty - power * 2) / SCOUT_RISK_DIVISOR + SCOUT_RISK_TRAIT * riskDirection(scout.getSkills());
    return std::max(SCOUT_RISK_MIN, std::min(SCOUT_RISK_MAX, risk));
}

static int pickWeightedVictim(const Team &team, const Roster &roster, std::mt19937 &rng)
{
    const std::vector<int> &ids = team.getMembersIds();
    std::vector<int> weights;
    weights.reserve(ids.size());
    for (int id : ids)
    {
        const std::vector<std::string> &skills = roster.findUnitById(id).getSkills();
        int weight = 1;
        int dir = riskDirection(skills);
        if (dir > 0)
            weight += VICTIM_WEIGHT_BONUS;
        else if (dir < 0)
            weight = std::max(1, weight - VICTIM_WEIGHT_PENALTY);
        weights.push_back(weight);
    }
    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    return ids[dist(rng)];
}

static int incidentChance(int floor, const Team &team, const Roster &roster)
{
    int chance = BASE_INCIDENT_CHANCE + static_cast<int>(floor * DEPTH_INCREMENT);
    for (int id : team.getMembersIds())
    {
        const std::vector<std::string> &skills = roster.findUnitById(id).getSkills();
        int dir = riskDirection(skills);
        if (dir > 0)
            chance += 5;
        else if (dir < 0)
            chance -= 3;
    }
    return std::min(chance, INCIDENT_CAP);
}

static const IncidentFlavor &pickIncidentFlavor(const Unit &victim, std::mt19937 &rng)
{
    const std::vector<IncidentFlavor> &pool =
        (riskDirection(victim.getSkills()) > 0) ? BOASTER_INCIDENTS : NEUTRAL_INCIDENTS;
    std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
    return pool[dist(rng)];
}

static bool pushesOn(const Team &team, const Roster &roster, bool found, int passes,
                     std::mt19937 &rng)
{
    int urge = found ? PUSH_BASE_FOUND : PUSH_BASE_SEARCHING;
    urge -= passes * PUSH_FATIGUE;

    for (int id : team.getMembersIds())
    {
        const std::vector<std::string> &skills = roster.findUnitById(id).getSkills();
        if (hasTrait(skills, "Greedy"))
            urge += PUSH_GREEDY;
        if (hasTrait(skills, "Curious"))
            urge += PUSH_CURIOUS;
        if (hasTrait(skills, "Cowardly"))
            urge -= PUSH_COWARDLY;
    }

    std::uniform_int_distribution<int> roll(1, 100);
    return roll(rng) <= urge;
}

// Which of a unit's traits speaks for them on this floor. Deliberately not random: a character
// who is Alert in one line and Superstitious in the next is not a character. Keyed on the id so
// two units with the same traits do not sound alike, and on the floor so the same unit is not
// stuck with one trait for their whole career -- consistent inside a scene, varied across one.
static const std::string &traitVoice(const Unit &u, int floor)
{
    const std::vector<std::string> &skills = u.getSkills();
    return skills[static_cast<size_t>(u.getId() + floor) % skills.size()];
}

static bool hurt(int unitId, Team &team, Roster &roster, GameState &state,
                 const std::vector<Injury> &injuries, int floor, const std::string &cause,
                 int dmgMin, int dmgMax, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> dmg(dmgMin, dmgMax);
    Unit &victim = roster.findUnitById(unitId);
    victim.takeDamage(dmg(rng));

    std::cout << "  " << victim.getName();
    if (!victim.getSkills().empty())
        std::cout << ", " << traitVoice(victim, floor) << " as ever, ";
    std::cout << "is wounded." << std::endl;

    if (!victim.isAlive())
    {
        std::cout << "  " << victim.getName();
        if (!victim.getSkills().empty())
            std::cout << ", " << traitVoice(victim, floor) << " to the end,";
        std::cout << " falls on floor " << floor << "." << std::endl;

        state.necropolis.addDeath(victim, floor, cause, state.incursionCount);
        roster.removeUnitById(unitId);
        team.purgeDeadMembers(roster);

        if (team.getMembersIds().empty())
        {
            std::cout << std::endl;
            std::cout << "The tower claims them all. No one returns." << std::endl;
            return false;
        }
    }
    else
    {
        std::uniform_int_distribution<int> injurieRoll(1, 100);
        if (injurieRoll(rng) <= WOUND_INJURY_CHANCE)
        {
            std::string injury = applyInjury(victim, injuries, rng);
            std::cout << "  " << victim.getName() << " carries the tower's mark for good -- "
                      << injury << "." << std::endl;
        }
    }
    return true;
}

static bool woundOne(Team &team, Roster &roster, GameState &state,
                     const std::vector<Injury> &injuries, int floor, const std::string &cause,
                     int dmgMin, int dmgMax, std::mt19937 &rng)
{
    int victimId = pickWeightedVictim(team, roster, rng);
    return hurt(victimId, team, roster, state, injuries, floor, cause, dmgMin, dmgMax, rng);
}

static void loseFloor(int floor, Team &team, Roster &roster, GameState &state,
                      const std::vector<Injury> &injuries, const std::string &cause,
                      int danger, std::mt19937 &rng)
{
    std::vector<int> ids = team.getMembersIds();

    for (int id : ids)
    {
        if (!hurt(id, team, roster, state, injuries, floor, cause, danger / 6, danger / 3, rng))
            return;
    }
    std::cout << "  They give up the floor and fall back." << std::endl;
}

void runScoutMission(int scoutId, Barracks &barracks, Roster &roster, GameState &state, std::mt19937 &rng)
{
    if (!roster.contains(scoutId))
    {
        std::cout << "No one by that id is on the roster." << std::endl;
        return;
    }

    int floor = state.highestFloor + 1;
    Objective objective = objectiveFor(floor, state, rng);
    Unit &scout = roster.findUnitById(scoutId);
    int risk = scoutRisk(scout, objective);

    std::cout << describeOdds(scout.getName(), risk) << std::endl;
    std::cout << "Send them up to floor " << floor << "? [1] Yes  [2] No" << std::endl;

    if (readChoice() != 1)
    {
        std::cout << "They stay where they are." << std::endl;
        return;
    }

    std::uniform_int_distribution<int> roll(1, 100);
    std::uniform_int_distribution<int> dmg(SCOUT_WOUND_MIN, SCOUT_WOUND_MAX);

    bool lost = roll(rng) <= risk;
    if (!lost)
    {
        scout.takeDamage(dmg(rng));
        lost = !scout.isAlive();
    }

    if (lost)
    {
        std::cout << scout.getName() << " goes up to floor " << floor
                  << " and does not come back." << std::endl;
        state.necropolis.addDeath(scout, floor, "lost scouting the floor alone",
                                  state.incursionCount);
        roster.removeUnitById(scoutId);
        barracks.purgeDead(roster);
        return;
    }

    Report report = scoutAhead(scout, objective, rng);
    state.floorReports[floor] = report;
    std::cout << describeReport(report) << std::endl;
}

// Resolves ONE floor: the encounter, the objective, and everything it costs. Returns true if
// the floor was cleared. Every early exit is a return, and the two that mean "cleared, and
// then it took them anyway" return true on purpose -- the floor was taken; what happened
// afterwards is a separate thing, and nothing here decides it.
//
// This is the seam. Today it still prints as it goes; when a sortie has to resolve with nobody
// watching a console (2d-6), what changes is inside here and inside hurt/loseFloor/awardFloor,
// not in runIncursion.
// A cleared floor still lets the tower take its cut. This used to live inside the "advances with
// ease" branch, which stopped being reachable the day all four objective types got a mechanic of
// their own -- so something we wanted was quietly dead for two days. It belongs to every clearing
// and not to one kind of it: you won, and on the way out something happened anyway.
// Always returns true: the floor WAS taken, whatever it cost afterwards.
static bool clearFloor(int floor, Team &team, Roster &roster, GameState &state,
                       const std::vector<Injury> &injuries, std::mt19937 &rng)
{
    awardFloor(floor, team, roster, state);

    int chance = incidentChance(floor, team, roster);
    std::uniform_int_distribution<int> incidentRoll(1, 100);
    if (team.getMembersIds().empty() || incidentRoll(rng) > chance)
        return true;

    int victimId = pickWeightedVictim(team, roster, rng);
    Unit &victim = roster.findUnitById(victimId);
    const IncidentFlavor &flavor = pickIncidentFlavor(victim, rng);

    std::uniform_int_distribution<int> fatalRoll(1, 100);
    if (fatalRoll(rng) > FATAL_CHANCE)
    {
        std::uniform_int_distribution<int> dmg(WOUND_DMG_MIN, WOUND_DMG_MAX);
        victim.takeDamage(dmg(rng));
        if (victim.isAlive())
        {
            std::cout << "  " << victim.getName() << " " << flavor.woundLine << "." << std::endl;

            std::uniform_int_distribution<int> injuryRoll(1, 100);
            if (injuryRoll(rng) <= WOUND_INJURY_CHANCE)
            {
                std::string injury = applyInjury(victim, injuries, rng);
                std::cout << "  " << victim.getName() << " will never be whole again -- "
                          << injury << "." << std::endl;
            }
            return true;
        }
    }

    std::cout << "  " << victim.getName() << " falls on floor " << floor << ", "
              << flavor.cause << "." << std::endl;
    state.necropolis.addDeath(victim, floor, flavor.cause, state.incursionCount);
    roster.removeUnitById(victimId);
    team.purgeDeadMembers(roster);

    if (team.getMembersIds().empty())
    {
        std::cout << std::endl;
        std::cout << "The tower claims them all. No one returns." << std::endl;
    }
    return true;
}

static bool resolveFloor(int floor, const Objective &objective, Team &team, Roster &roster,
                         GameState &state, const std::vector<Encounter> &encounters,
                         const std::vector<Injury> &injuries, std::mt19937 &rng)
{
    int fit = teamFit(team, roster, objective.type);
    int danger = objective.difficulty;
    int traitMod = 0;

    std::uniform_int_distribution<size_t> pickEnc(0, encounters.size() - 1);
    const Encounter &enc = encounters[pickEnc(rng)];
    std::cout << "Floor " << floor << ": " << enc.description << "." << std::endl;
    std::cout << "Objective: " << describeObjective(objective) << std::endl;
    std::cout << "  " << describeFit(fit) << std::endl;

    auto rep = state.floorReports.find(floor);
    if (rep != state.floorReports.end() && rep->second.sawObjective && rep->second.claimed.type != objective.type)
        std::cout << "  " << describeMisreport(rep->second) << std::endl;

    std::vector<std::pair<int, const TraitEvent *>> candidates;
    for (int id : team.getMembersIds())
    {
        const Unit &u = roster.findUnitById(id);
        for (const TraitEvent &ev : DAMAGE_TRAIT_EVENTS)
            if (hasTrait(u.getSkills(), ev.trait))
                candidates.push_back({id, &ev});
    }

    if (!candidates.empty())
    {
        std::uniform_int_distribution<int> coin(0, 1);
        if (coin(rng) == 1)
        {
            auto selected = candidates[std::uniform_int_distribution<size_t>(0, candidates.size() - 1)(rng)];
            const TraitEvent *event = selected.second;

            const Unit &actor = roster.findUnitById(selected.first);
            std::cout << "  " << actor.getName() << ", " << event->trait << " as ever, "
                      << pickRandom(event->deeds, rng) << "." << std::endl;

            traitMod += event->attackModifier;
        }
    }


    if (objective.type == ObjectiveType::Hold)
    {
        int failures = 0;
        bool wiped = false;
        std::string lastLine; // fed back to pickRandom so no two rounds read the same

        for (int round = 1; round <= objective.rounds; round++)
        {
            int roll = floorRoll(team, roster, objective.type, traitMod, danger, rng);

            if (roll >= danger)
            {
                lastLine = pickRandom(HOLD_HELD, rng, lastLine);
                std::cout << "  Round " << round << ": " << lastLine
                          << "." << std::endl;
            }
            else
            {
                lastLine = pickRandom(HOLD_GAVE_GROUND, rng, lastLine);
                std::cout << "  Round " << round << ": " << lastLine
                          << "." << std::endl;
                ++failures;
                if (!woundOne(team, roster, state, injuries, floor, enc.cause,
                              HOLD_DMG_MIN, HOLD_DMG_MAX, rng))
                {
                    wiped = true;
                    break;
                }
            }
        }

        if (wiped)
            return false;

        if (failures * 2 > objective.rounds)
        {
            std::cout << "  They are pushed off the floor." << std::endl;
            loseFloor(floor, team, roster, state, injuries, enc.cause, danger, rng);
            return false;
        }

        std::cout << (failures == 0 ? "  The line never broke."
                                    : "  The line held, and it cost them.")
                  << std::endl;
        return clearFloor(floor, team, roster, state, injuries, rng);
    }
    else if (objective.type == ObjectiveType::Slay)
    {
        std::string lastLine; // as above: never the same line twice running
        int enemyHpStart = danger * 3 / 2;
        int enemyHp = enemyHpStart;
        int failures = 0;
        bool wiped = false;

        while (enemyHp > 0 && failures < SLAY_COUNTERS_ENDURED)
        {
            int roll = floorRoll(team, roster, objective.type, traitMod, danger, rng);
            int margin = roll - danger;

            if (margin >= 0)
            {
                std::uniform_int_distribution<int> critRoll(1, 100);
                bool crit = critRoll(rng) <= SLAY_CRIT_CHANCE;

                int hit = danger / 2 + margin;
                if (crit)
                    hit += enemyHpStart / 2;
                enemyHp -= hit;

                const std::vector<std::string> &bank =
                    (crit || margin >= luckSpan(danger)) ? SLAY_CRUSHED : SLAY_LANDED;
                lastLine = pickRandom(bank, rng, lastLine);
                std::cout << "  " << lastLine << "." << std::endl;
            }
            else
            {
                ++failures;
                const std::vector<std::string> &bank =
                    (margin >= -luckSpan(danger) / 2) ? SLAY_BLOCKED : SLAY_MISSED;
                lastLine = pickRandom(bank, rng, lastLine);
                std::cout << "  " << lastLine << "." << std::endl;

                if (!woundOne(team, roster, state, injuries, floor, enc.cause,
                              SLAY_DMG_MIN, SLAY_DMG_MAX, rng))
                {
                    wiped = true;
                    break;
                }
            }
        }

        if (wiped)
            return false;

        if (enemyHp > 0)
        {
            std::cout << "  " << describeSurvivor(enemyHp, enemyHpStart) << std::endl;
            loseFloor(floor, team, roster, state, injuries, enc.cause, danger, rng);
            return false;
        }

        std::cout << (failures == 0 ? "  It falls without landing a blow."
                                    : "  It falls, and they have paid for it.")
                  << std::endl;
        return clearFloor(floor, team, roster, state, injuries, rng);
    }
    else if (objective.type == ObjectiveType::Retrieve)
    {
        int passes = 0;
        int exposure = 0;
        int extra = 0;
        bool found = false;
        std::uniform_int_distribution<int> notice(1, 100);
        std::string lastLine; // a search can run nine passes; repeats show

        while (true)
        {
            passes++;
            exposure += EXPOSURE_STEP;

            if (floorRoll(team, roster, objective.type, traitMod, danger, rng) >= danger)
            {
                if (!found)
                {
                    found = true;
                    lastLine = pickRandom(RETRIEVE_FOUND, rng, lastLine);
                    std::cout << "  " << lastLine << "." << std::endl;
                }
                else
                {
                    extra += floor / 2;
                    lastLine = pickRandom(RETRIEVE_MORE, rng, lastLine);
                    std::cout << "  " << lastLine << "." << std::endl;
                }
            }
            else
            {
                lastLine = pickRandom(RETRIEVE_NOTHING, rng, lastLine);
                std::cout << "  " << lastLine << "." << std::endl;
            }

            if (exposure >= EXPOSURE_CAUGHT)
            {
                std::cout << "  They stayed too long. What lives here comes for them." << std::endl;
                loseFloor(floor, team, roster, state, injuries, enc.cause, danger, rng);
                return false;
            }

            if (notice(rng) <= exposure)
            {
                lastLine = pickRandom(RETRIEVE_NOTICED, rng, lastLine);
                std::cout << "  " << lastLine << "." << std::endl;
                if (!hurt(pickWeightedVictim(team, roster, rng), team, roster, state,
                          injuries, floor, enc.cause, danger / 6, danger / 3, rng))
                    return found;
            }

            if (!pushesOn(team, roster, found, passes, rng))
                break;
        }
        if (!found)
        {
            std::cout << "  They give it up and go. Whatever the floor is keeping, it keeps."
                      << std::endl;
            return false;
        }

        if (extra > 0)
        {
            state.essence += extra;
            std::cout << "  They come out with more than they came for: " << extra
                      << " essence." << std::endl;
        }
        return clearFloor(floor, team, roster, state, injuries, rng);
    }
    else if (objective.type == ObjectiveType::Rescue)
    {
        int reach = danger / 4; // a party at parity covers this about half the time; a weaker one does not
        int progress = 0;
        bool wiped = false;
        std::string lastLine;

        for (int round = 1; round <= objective.rounds; round++)
        {
            int margin = floorRoll(team, roster, objective.type, traitMod, danger, rng) - danger;

            if (margin > 0) {
                progress += margin;
                lastLine = pickRandom(RESCUE_CLOSER, rng, lastLine);
                std::cout << "  " << lastLine << "." << std::endl;
            
                if (progress >= reach)
                    break;
            }
            else 
            {
                lastLine = pickRandom(RESCUE_HELD, rng, lastLine);
                std::cout << "  " << lastLine << "." << std::endl;
                if (!woundOne(team, roster, state, injuries, floor, enc.cause, 
                    HOLD_DMG_MIN, HOLD_DMG_MAX, rng))
                {
                    wiped = true;
                    break;
                }
            }
        }
        if (wiped)
            return false;
        
        if (progress < reach)
        {
            std::cout << "  They are still a corridor away when the sound stops." << std::endl;
            return false;
        }

        std::cout << "  They reach the captive and cut them loose." << std::endl;
        return clearFloor(floor, team, roster, state, injuries, rng);
    }
    return true;
}

bool runIncursion(Team &team, Roster &roster, GameState &state, const std::vector<Encounter> &encounters, const std::vector<Injury> &injuries, std::mt19937 &rng)
{
    if (team.getMembersIds().empty())
    {
        std::cout << "No units in the team. Assemble a team first." << std::endl;
        return false;
    }

    std::cout << std::endl;
    team.printTeam(roster);

    int startFloor = 1;
    if (state.highestFloor > 0)
    {
        std::cout << "Start from floor? [1-" << state.highestFloor + 1
                  << "]" << std::endl;
        startFloor = readChoice();
        if (startFloor < 1 || startFloor > state.highestFloor + 1)
            startFloor = 1;
    }

    Objective objective = objectiveFor(startFloor, state, rng);

    auto scouted = state.floorReports.find(startFloor);
    if (startFloor <= state.highestFloor)
    {
        printForecast(teamPower(team, roster), objective.difficulty);
    }
    else if (scouted != state.floorReports.end() && scouted->second.sawDanger)
    {
        std::cout << scouted->second.scout << " went up and came back. By their account:" << std::endl;
        if (scouted->second.sawObjective)
            std::cout << "  " << describeObjective(scouted->second.claimed) << std::endl;
        printForecast(teamPower(team, roster) + scouted->second.bias, objective.difficulty);
    }
    else
    {
        std::cout << "No one has stood on floor " << startFloor
                  << ". What waits there is a guess." << std::endl;
    }

    std::cout << "Send them in? [1] Yes  [2] Hold back" << std::endl;
    if (readChoice() != 1)
    {
        std::cout << "They stay in the doorway. Nothing is spent." << std::endl;
        return false;
    }

    state.incursionCount++;
    std::cout << std::endl;
    std::cout << "=== Sortie " << state.incursionCount << ": floor " << startFloor
              << " ===" << std::endl;

    std::cout << std::endl;

    bool cleared = resolveFloor(startFloor, objective, team, roster, state, encounters,
                                injuries, rng);
    if (cleared && startFloor > state.highestFloor)
        state.highestFloor = startFloor;

    team.purgeDeadMembers(roster);

    std::cout << std::endl;
    std::cout << (cleared ? "=== Floor taken ===" : "=== Floor held them ===")
              << std::endl;
    std::cout << "Tower record: floor " << state.highestFloor
              << "  |  Essence: " << state.essence << std::endl;
    if (!team.getMembersIds().empty())
    {
        std::cout << "They come back down still carrying their wounds. Only rest away from"
                  << " the tower will mend them." << std::endl;
        team.printTeam(roster);
    }
    return true;
}

std::vector<Encounter> loadEncounters(const std::string &path)
{
    std::vector<Encounter> encounters;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        size_t pos = line.find('|');
        if (pos == std::string::npos)
            throw std::runtime_error("Malformed line in " + path + ": " + line);

        std::string description = line.substr(0, pos);
        std::string cause = line.substr(pos + 1);

        encounters.push_back({description, cause});
    }
    if (encounters.empty())
        throw std::runtime_error("Encounter bank is empty or file not found: " + path);
    return encounters;
}
