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

static const int MAX_LUCK = 30;
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
    if (power >= danger * 12 / 10)
        std::cout << COLOR_GREEN << "The way up looks clear." << COLOR_RESET << std::endl;
    else if (power + MAX_LUCK >= danger * 12 / 10)
        std::cout << COLOR_YELLOW << "The air grows heavier." << COLOR_RESET << std::endl;
    else if (power + MAX_LUCK >= danger)
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

static int riskDirection(const std::vector<std::string> &skills)
{
    if (std::find(skills.begin(), skills.end(), "Reckless") != skills.end() ||
        std::find(skills.begin(), skills.end(), "Boaster") != skills.end())
        return 1;
    if (std::find(skills.begin(), skills.end(), "Alert") != skills.end())
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

static bool woundOne(Team &team, Roster &roster, GameState &state,
                     const std::vector<Injury> &injuries, int floor, const std::string &cause,
                     int dmgMin, int dmgMax, std::mt19937 &rng)
{
    int victimId = pickWeightedVictim(team, roster, rng);
    std::uniform_int_distribution<int> dmg(dmgMin, dmgMax);
    Unit &victim = roster.findUnitById(victimId);
    victim.takeDamage(dmg(rng));

    std::cout << "  " << victim.getName();
    if (!victim.getSkills().empty())
        std::cout << ", " << pickRandom(victim.getSkills(), rng) << " as ever, ";
    std::cout << "is wounded." << std::endl;

    if (!victim.isAlive())
    {
        std::cout << "  " << victim.getName();
        if (!victim.getSkills().empty())
            std::cout << ", " << pickRandom(victim.getSkills(), rng) << " to the end,";
        std::cout << " falls on floor " << floor << ". The others mourn and press on." << std::endl;

        state.necropolis.addDeath(victim, floor, cause, state.incursionCount);
        roster.removeUnitById(victimId);
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

static void loseFloor(int floor, Team &team, Roster &roster, GameState &state,
                      const std::string &cause, std::mt19937 &rng)
{
    const std::vector<int> &ids = team.getMembersIds();
    std::uniform_int_distribution<size_t> pick(0, ids.size() - 1);
    int victimId = ids[pick(rng)];
    const Unit &fallen = roster.findUnitById(victimId);

    std::cout << "  " << fallen.getName();
    if (!fallen.getSkills().empty())
        std::cout << ", " << pickRandom(fallen.getSkills(), rng) << " to the end,";
    std::cout << " falls on floor " << floor << ". The others retreat." << std::endl;

    state.necropolis.addDeath(fallen, floor, cause, state.incursionCount);
    roster.removeUnitById(victimId);
    team.purgeDeadMembers(roster);

    if (team.getMembersIds().empty())
    {
        std::cout << std::endl;
        std::cout << "The tower claims them all. No one returns." << std::endl;
    }
}

void runScoutMission(int scoutId, Team &team, Roster &roster, GameState &state, std::mt19937 &rng)
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
        team.purgeDeadMembers(roster);
        return;
    }

    Report report = scoutAhead(scout, objective, rng);
    state.floorReports[floor] = report;
    std::cout << describeReport(report) << std::endl;
}

void runIncursion(Team &team, Roster &roster, GameState &state, const std::vector<Encounter> &encounters, const std::vector<Injury> &injuries, std::mt19937 &rng)
{
    if (team.getMembersIds().empty())
    {
        std::cout << "No units in the team. Assemble a team first." << std::endl;
        return;
    }

    state.incursionCount++;

    std::cout << std::endl;
    std::cout << "=== Incursion " << state.incursionCount << " ===" << std::endl;
    team.printTeam(roster);

    int startFloor = 1;
    if (state.highestFloor > 0)
    {
        std::cout << "Start from floor? [1-" << state.highestFloor + 1
                  << "] (toll: 1 essence per floor above the first)" << std::endl;
        startFloor = readChoice();
        if (startFloor < 1 || startFloor > state.highestFloor + 1)
            startFloor = 1;
    }

    int toll = startFloor - 1;
    if (toll > state.essence)
    {
        std::cout << "Not enough essence for floor " << startFloor << " (" << state.essence
                  << "/" << toll << "). The tower lets you in at floor 1." << std::endl;
        startFloor = 1;
        toll = 0;
    }
    if (toll > 0)
    {
        state.essence -= toll;
        std::cout << "The tower takes its toll: " << toll << " essence." << std::endl;
    }

    std::cout << std::endl;

    int highestFloorThisRun = 0;
    std::uniform_int_distribution<int> luck(0, MAX_LUCK);
    Objective objective = objectiveFor(startFloor, state, rng);

    for (int floor = startFloor;; floor++)
    {
        int fit = teamFit(team, roster, objective.type);
        int power = teamPower(team, roster) + fit;
        int danger = objective.difficulty;
        int attack = power + luck(rng);
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
                if (std::find(u.getSkills().begin(), u.getSkills().end(), ev.trait) != u.getSkills().end())
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
        attack += traitMod;

        if (objective.type == ObjectiveType::Hold)
        {
            int failures = 0;
            bool wiped = false;

            for (int round = 1; round <= objective.rounds; round++)
            {
                int roll = teamPower(team, roster) + teamFit(team, roster, objective.type) + traitMod + luck(rng);

                if (roll >= danger)
                    std::cout << "  Round " << round << ": " << pickRandom(HOLD_HELD, rng)
                              << "." << std::endl;
                else
                {
                    std::cout << "  Round " << round << ": " << pickRandom(HOLD_GAVE_GROUND, rng)
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
                break;

            if (failures * 2 > objective.rounds)
            {
                std::cout << "  They are pushed off the floor." << std::endl;
                loseFloor(floor, team, roster, state, enc.cause, rng);
                break;
            }

            std::cout << (failures == 0 ? "  The line never broke."
                                        : "  The line held, and it cost them.")
                      << std::endl;
            awardFloor(floor, team, roster, state);

            highestFloorThisRun = floor;
            if (floor > state.highestFloor)
                state.highestFloor = floor;
        }
        else if (objective.type == ObjectiveType::Slay) 
        {
            int enemyHpStart = danger * 3 / 2;
            int enemyHp = enemyHpStart;
            int failures = 0;
            bool wiped = false;

            while (enemyHp > 0 && failures < SLAY_COUNTERS_ENDURED)
            {
                int roll = teamPower(team, roster) + teamFit(team, roster, objective.type)
                            + traitMod + luck(rng);
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
                        (crit || margin >= MAX_LUCK) ? SLAY_CRUSHED : SLAY_LANDED;
                    std::cout << "  " << pickRandom(bank, rng) << "." << std::endl;
                }
                else
                {
                    ++failures;
                    const std::vector<std::string> &bank =
                        (margin >= -MAX_LUCK / 2) ? SLAY_BLOCKED : SLAY_MISSED;
                    std::cout << "  " << pickRandom(bank, rng) << "." << std::endl;

                    if (!woundOne(team, roster, state, injuries, floor, enc.cause,
                                    SLAY_DMG_MIN, SLAY_DMG_MAX, rng))
                    {
                        wiped = true;
                        break;
                    }
                }
            }
            
            if (wiped)
                break;
            
            if (enemyHp > 0)
            {
                std::cout << "  " << describeSurvivor(enemyHp, enemyHpStart) << std::endl;
                loseFloor(floor, team, roster, state, enc.cause, rng);
                break;
            }

            std::cout << (failures == 0 ? "  It falls without landing a blow."
                                        : "  It falls, and they have paid for it.") << std::endl;
            awardFloor(floor, team, roster, state);
            
            highestFloorThisRun = floor;
            if (floor > state.highestFloor)
                state.highestFloor = floor;
        }
        else if (attack >= danger * 12 / 10)
        {
            std::cout << "  The team advances with ease." << std::endl;
            awardFloor(floor, team, roster, state);

            int chance = incidentChance(floor, team, roster);
            std::uniform_int_distribution<int> incidentRoll(1, 100);
            if (incidentRoll(rng) <= chance)
            {
                int victimId = pickWeightedVictim(team, roster, rng);
                Unit &victim = roster.findUnitById(victimId);

                std::uniform_int_distribution<int> fatalRoll(1, 100);
                if (fatalRoll(rng) <= FATAL_CHANCE)
                {
                    const IncidentFlavor &flavor = pickIncidentFlavor(victim, rng);
                    std::cout << "  " << victim.getName() << " falls on floor " << floor << ", " << flavor.cause << "." << std::endl;
                    state.necropolis.addDeath(victim, floor, flavor.cause, state.incursionCount);
                    roster.removeUnitById(victimId);
                    team.purgeDeadMembers(roster);

                    if (team.getMembersIds().empty())
                    {
                        std::cout << std::endl;
                        std::cout << "The tower claims them all. No one returns." << std::endl;
                        break;
                    }
                }
                else
                {
                    const IncidentFlavor &flavor = pickIncidentFlavor(victim, rng);
                    std::uniform_int_distribution<int> dmg(15, 40);
                    victim.takeDamage(dmg(rng));

                    if (!victim.isAlive())
                    {
                        std::cout << "  " << victim.getName() << " falls on floor " << floor
                                  << ", " << flavor.cause << "." << std::endl;

                        state.necropolis.addDeath(victim, floor, flavor.cause, state.incursionCount);
                        roster.removeUnitById(victimId);
                        team.purgeDeadMembers(roster);

                        if (team.getMembersIds().empty())
                        {
                            std::cout << std::endl;
                            std::cout << "The tower claims them all. No one returns." << std::endl;
                            break;
                        }
                    }
                    else
                    {
                        std::cout << "  " << victim.getName() << " " << flavor.woundLine << "." << std::endl;

                        std::uniform_int_distribution<int> injurieRoll(1, 100);
                        if (injurieRoll(rng) <= WOUND_INJURY_CHANCE)
                        {
                            std::string injury = applyInjury(victim, injuries, rng);
                            std::cout << "  " << victim.getName() << " will never be whole again -- " << injury << "." << std::endl;
                        }
                    }
                }
            }

            highestFloorThisRun = floor;
            if (floor > state.highestFloor)
                state.highestFloor = floor;
        }
        else if (attack >= danger)
        {

            std::cout << "  The team advances with difficulty." << std::endl;
            awardFloor(floor, team, roster, state);

            if (!woundOne(team, roster, state, injuries, floor, enc.cause,
                          WOUND_DMG_MIN, WOUND_DMG_MAX, rng))
                break;

            highestFloorThisRun = floor;
            if (floor > state.highestFloor)
                state.highestFloor = floor;
        }
        else
        {
            std::cout << "  The tower overwhelms the team." << std::endl;
            loseFloor(floor, team, roster, state, enc.cause, rng);

            break;
        }

        Objective nextObjective = objectiveFor(floor + 1, state, rng);
        int nextDanger = nextObjective.difficulty;
        int currentPower = teamPower(team, roster);

        printForecast(currentPower, nextDanger);

        std::cout << "Climb to floor " << floor + 1 << "? [1] Yes  [2] Return" << std::endl;
        if (readChoice() != 1)
        {
            std::cout << "The team descends with their spoils and their lives." << std::endl;
            break;
        }
        objective = nextObjective;
    }

    team.purgeDeadMembers(roster);

    std::cout << std::endl;
    std::cout << "=== Incursion " << state.incursionCount << " over ===" << std::endl;
    std::cout << "Highest floor this run: " << highestFloorThisRun
              << "  |  Tower record: " << state.highestFloor
              << "  |  Essence: " << state.essence << std::endl;
    if (!team.getMembersIds().empty())
    {
        std::cout << "They come back down still carrying their wounds. Only rest away from"
                  << " the tower will mend them." << std::endl;
        team.printTeam(roster);
    }
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
