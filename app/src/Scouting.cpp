#include "Scouting.h"

#include <algorithm>

static const int SCOUT_SPREAD = 15;  // one floor: how wrong an ordinary scout can be, either way
static const int SCOUT_SKEW = 15;    // one more floor, but always the same way: bias, not error

static bool hasTrait(const std::vector<std::string> &traits, const std::string &trait) {
    return traits.end() != std::find(traits.begin(), traits.end(), trait);
}

Report scoutAhead(const Unit &scout, const Objective &objective, std::mt19937 &rng) {
    Report report;
    report.scout = scout.getName();
    report.scoutId = scout.getId();
    report.sawObjective = true;
    report.claimed = objective;
    report.sawDanger = true;
    int spread = SCOUT_SPREAD;
    if (hasTrait(scout.getSkills(), "Alert"))
        spread -= SCOUT_SPREAD / 2;
    std::uniform_int_distribution<int> jitter(-spread, spread);
    report.bias = jitter(rng);
    if (hasTrait(scout.getSkills(), "Boaster"))
        report.bias += SCOUT_SKEW;
    if (hasTrait(scout.getSkills(), "Cowardly"))
        report.bias -= SCOUT_SKEW;
    return report;
}

std::string describeReport(const Report &report) {
    if (!report.sawObjective)
        return report.scout + " slips ahead and comes back with nothing worth telling.";

    std::string line = report.scout + " slips ahead and comes back. Their read: "
                     + describeObjective(report.claimed);

    if (!report.sawDanger)
        line += " They never got near enough to judge the odds.";

    return line;
}

std::string describeOdds(const std::string &scout, int risk) {
    if (risk <= 10) return scout + " looks up the stairwell and reckons they can manage it.";
    if (risk <= 25) return scout + " looks up the stairwell and says nothing for a moment.";
    if (risk <= 40) return scout + " looks up the stairwell and does not like their odds.";
    return scout + " looks up the stairwell and does not expect to come back.";
}

std::string describeMisreport(const Report &report) {
    return report.scout + " had described something else entirely.";
}