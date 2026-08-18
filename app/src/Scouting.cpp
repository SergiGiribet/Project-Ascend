#include "Scouting.h"

Report scoutAhead(const Unit &scout, const Objective &objective, std::mt19937 &rng) {
    Report report;
    report.scout = scout.getName();
    report.scoutId = scout.getId();
    report.sawObjective = true;
    report.claimed = objective;
    report.sawDanger = true;
    report.bias = 0;
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

std::string describeMisreport(const Report &report) {
    return report.scout + " had described something else entirely.";
}