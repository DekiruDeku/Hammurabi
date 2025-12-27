#pragma once

#include <string>

// Reads tunable parameters from a simple key=value file.
// Keys are compatible with the provided game_rules.txt.
struct GameRules {
    // Initial parameters
    int initialPopulation = -1;
    double initialGrainBushels = -1.0;
    int initialLandAcres = -1;

    // Land price per acre
    int landPriceMin = -1;
    int landPriceMax = -1;

    // Consumption and work limits
    int bushelsPerPersonPerYear = -1;  // 20
    int acresPerPersonMax = -1;        // 10

    // Seeds
    double seedsBushelsPerAcre = -1.0; // 0.5

    // Harvest yield per acre
    int yieldPerAcreMin = -1; // 1
    int yieldPerAcreMax = -1; // 6

    // Rats: eat random integer in [0; ratsMaxFraction * grain]
    double ratsMaxFraction = -1.0;     // 0.07

    // Plague probability in [0;1]
    double plagueProbability = -1.0;   // 0.15

    // Immigration clamp
    int immigrantsMin = -1; // 0
    int immigrantsMax = -1; // 50

    // Early loss if more than this fraction starves in one year
    double starvationLossFraction = -1.0; // 0.45

    // Number of years
    int totalYears = -1; // 10

    // Final assessment thresholds
    int pBadLower = -1;
    int lBadUpper = -1;
    int pOkLower = -1;
    int lOkUpper = -1;
    int pGoodLower = -1;
    int lGoodUpper = -1;

    bool loadFromFile(const std::string& filename);
    bool isFilled() const;
};
