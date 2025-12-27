#include "GameRules.hpp"

#include <fstream>
#include <sstream>
#include <string>

static double parseDoubleFlexible(const std::string& raw) {
    std::string s = raw;
    for (char& c : s) {
        if (c == ',') c = '.';
    }
    return std::stod(s);
}

static int parseInt(const std::string& s) {
    return std::stoi(s);
}

bool GameRules::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string key, value;
        if (!std::getline(ss, key, '=')) continue;
        if (!std::getline(ss, value)) continue;

        while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) value.pop_back();
        while (!key.empty() && (key.front() == ' ' || key.front() == '\t')) key.erase(key.begin());
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.erase(value.begin());

        if (key == "initial_population") initialPopulation = parseInt(value);
        else if (key == "initial_wheat") initialGrainBushels = parseDoubleFlexible(value);
        else if (key == "initial_city_size") initialLandAcres = parseInt(value);

        else if (key == "land_price_min") landPriceMin = parseInt(value);
        else if (key == "land_price_max") landPriceMax = parseInt(value);

        else if (key == "resident_consumption") bushelsPerPersonPerYear = parseInt(value);
        else if (key == "resident_efficiency") acresPerPersonMax = parseInt(value);

        else if (key == "seeds_consumption") seedsBushelsPerAcre = parseDoubleFlexible(value);

        else if (key == "land_efficiency_min") yieldPerAcreMin = parseInt(value);
        else if (key == "land_efficiency_max") yieldPerAcreMax = parseInt(value);

        else if (key == "rats_consumption_rate_max") ratsMaxFraction = parseDoubleFlexible(value);

        else if (key == "plague_probability") plagueProbability = parseDoubleFlexible(value);

        else if (key == "arrivals_number_min") immigrantsMin = parseInt(value);
        else if (key == "arrivals_number_max") immigrantsMax = parseInt(value);

        else if (key == "death_percentage_for_loss") starvationLossFraction = parseDoubleFlexible(value);

        else if (key == "rounds_number") totalYears = parseInt(value);

        else if (key == "dead_percentage_lower_limit_bad") pBadLower = parseInt(value);
        else if (key == "lands_per_resident_bad") lBadUpper = parseInt(value);
        else if (key == "dead_percentage_lower_limit_ok") pOkLower = parseInt(value);
        else if (key == "lands_per_resident_ok") lOkUpper = parseInt(value);
        else if (key == "dead_percentage_lower_limit_good") pGoodLower = parseInt(value);
        else if (key == "lands_per_resident_good") lGoodUpper = parseInt(value);

    }

    return isFilled();
}

bool GameRules::isFilled() const {
    return initialPopulation > 0 && initialGrainBushels >= 0.0 && initialLandAcres >= 0 &&
        landPriceMin > 0 && landPriceMax >= landPriceMin &&
        bushelsPerPersonPerYear > 0 && acresPerPersonMax > 0 &&
        seedsBushelsPerAcre > 0.0 &&
        yieldPerAcreMin > 0 && yieldPerAcreMax >= yieldPerAcreMin &&
        ratsMaxFraction >= 0.0 && ratsMaxFraction <= 1.0 &&
        plagueProbability >= 0.0 && plagueProbability <= 1.0 &&
        immigrantsMin >= 0 && immigrantsMax >= immigrantsMin &&
        starvationLossFraction > 0.0 && starvationLossFraction <= 1.0 &&
        totalYears > 0 &&
        pBadLower >= 0 && lBadUpper >= 0 &&
        pOkLower >= 0 && lOkUpper >= 0 &&
        pGoodLower >= 0 && lGoodUpper >= 0;
}
