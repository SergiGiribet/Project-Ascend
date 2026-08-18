#include "Scouting.h"

Report scoutAhead(const Unit &scout, const Objective &objective, std::mt19937 &rng) {
    Report report;
    report.scout = scout.getName();
    report.sawObjective = true;
    report.claimed = objective;
    report.sawDanger = true;
    report.bias = 0;
    return report;
}

std::string describeReport(const Report &report) {
    if (!report.sawObjective)
        return report.scout + " slips ahead and comes back with nothing useful.";

    return report.scout + " slips ahead and comes back: " + describeObjective(report.claimed);
}