#pragma once

#include <cmath>

// Single source of truth for the whole game progress.
// All input is integer, but grain can become fractional because 0.5 bushels/acre are spent on seeds.
struct GameState {
    // Current year (round) to be played. Starts from 1.
    int year = 1;

    // Core resources
    int population = 100;
    double grainBushels = 2800.0;   // grain in granaries
    int landAcres = 1000;

    // "Last year" report values (for the report at the start of a round)
    int starvedLastYear = 0;
    int immigrantsLastYear = 0;
    bool plagueLastYear = false;
    int yieldPerAcreLastYear = 0;
    int harvestTotalLastYear = 0;
    int ratsAteLastYear = 0;

    // Current year market state
    int landPriceThisYear = 0; // generated at the start of each year
    bool awaitingPlayerDecisions = false; // if true, resume must NOT re-roll land price

    // Scoring
    int yearsCompleted = 0;            // how many years are fully processed
    double starvationPercentSum = 0.0; // sum of starvation percentages for each completed year
};

inline bool IsValidSave(const GameState& s) {
    if (s.year < 1) return false;
    if (s.population <= 0) return false;
    if (s.landAcres < 0) return false;
    if (!std::isfinite(s.grainBushels) || s.grainBushels < 0.0) return false;
    return true;
}
