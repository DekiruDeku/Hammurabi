#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>

#include "GameRules.hpp"
#include "GameState.hpp"
#include "SaveManager.hpp"

namespace {

class Rng {
public:
    Rng() : engine_(std::random_device{}()) {}

    int intInRange(int lo, int hi) {
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(engine_);
    }

    bool chance(double probability) {
        std::bernoulli_distribution dist(probability);
        return dist(engine_);
    }

private:
    std::mt19937 engine_;
};

std::string readLineTrimmed() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "\nInput ended.\n";
        std::exit(0);
    }
    auto is_ws = [](unsigned char c) { return c == ' ' || c == '\t' || c == '\r'; };
    while (!line.empty() && is_ws(static_cast<unsigned char>(line.front()))) line.erase(line.begin());
    while (!line.empty() && is_ws(static_cast<unsigned char>(line.back()))) line.pop_back();
    return line;
}

int readIntNonNegative(const std::string& prompt) {
    for (;;) {
        std::cout << prompt;
        std::string s = readLineTrimmed();
        try {
            size_t pos = 0;
            int v = std::stoi(s, &pos, 10);
            if (pos != s.size()) throw std::invalid_argument("junk");
            if (v < 0) throw std::out_of_range("negative");
            return v;
        } catch (...) {
            std::cout << "Please enter an integer >= 0.\n";
        }
    }
}

bool askYesNo(const std::string& prompt) {
    for (;;) {
        std::cout << prompt;
        std::string s = readLineTrimmed();
        if (s.empty()) continue;
        char c = static_cast<char>(std::tolower(static_cast<unsigned char>(s[0])));
        if (c == 'y') return true;
        if (c == 'n') return false;
        std::cout << "Please enter Y/N.\n";
    }
}

std::string formatGrain(double bushels) {
    double rounded = std::round(bushels * 2.0) / 2.0;
    if (std::fabs(rounded - std::round(rounded)) < 1e-9) {
        return std::to_string(static_cast<long long>(std::llround(rounded)));
    }
    std::string s = std::to_string(rounded);
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

//Игровая логика

void initNewGame(GameState& s, const GameRules& r) {
    s.year = 1;
    s.population = r.initialPopulation;
    s.grainBushels = r.initialGrainBushels;
    s.landAcres = r.initialLandAcres;

    s.starvedLastYear = 0;
    s.immigrantsLastYear = 0;
    s.plagueLastYear = false;
    s.yieldPerAcreLastYear = 0;
    s.harvestTotalLastYear = 0;
    s.ratsAteLastYear = 0;

    s.landPriceThisYear = 0;
    s.awaitingPlayerDecisions = false;

    s.yearsCompleted = 0;
    s.starvationPercentSum = 0.0;
}

void printRoundHeader(const GameState& s) {
    std::cout << "\n========================================\n";
    std::cout << "Year " << s.year << " of your rule\n";
    std::cout << "----------------------------------------\n";
}

void printReport(const GameState& s) {
    printRoundHeader(s);

    if (s.year == 1) {
        std::cout << "You have just taken the throne.\n";
    } else {
        if (s.starvedLastYear > 0) {
            std::cout << s.starvedLastYear << " people starved to death last year.\n";
        }
        if (s.immigrantsLastYear > 0) {
            std::cout << s.immigrantsLastYear << " people moved into the city.\n";
        }
        if (s.plagueLastYear) {
            std::cout << "A plague killed half the population!\n";
        }

        if (s.harvestTotalLastYear > 0) {
            std::cout << "We harvested " << s.harvestTotalLastYear
                      << " bushels of grain (" << s.yieldPerAcreLastYear
                      << " per acre).\n";
        }
        if (s.ratsAteLastYear > 0) {
            std::cout << "Rats destroyed " << s.ratsAteLastYear << " bushels of grain.\n";
        }
    }

    std::cout << "Current population: " << s.population << "\n";
    std::cout << "Grain in storage: " << formatGrain(s.grainBushels) << " bushels\n";
    std::cout << "Land owned: " << s.landAcres << " acres\n";
    std::cout << "Land price this year: " << s.landPriceThisYear << " bushels per acre\n";
}

bool maybeSaveAndQuitAtRoundStart(const SaveManager& saves, const GameState& s) {
    std::cout << "\nAt the start of the year you may quit and save your progress.\n";
    std::cout << "Save and quit? (Y/N, empty = N): ";
    std::string ans = readLineTrimmed();
    if (ans.empty()) return false;
    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(ans[0])));
    if (c == 'y') {
        saves.save(s);
        std::cout << "Progress saved. See you!\n";
        return true;
    }
    return false;
}

bool playOneYear(GameState& s, const GameRules& r, Rng& rng, const SaveManager& saves) {
    if (!s.awaitingPlayerDecisions) {
        s.landPriceThisYear = rng.intInRange(r.landPriceMin, r.landPriceMax);
        s.awaitingPlayerDecisions = true;
    }

    printReport(s);
    if (maybeSaveAndQuitAtRoundStart(saves, s)) {
        return false;
    }

    std::cout << "\nWhat do you wish to do this year?\n";

    // 1) Покупка земли
    int acresToBuy = 0;
    int acresToSell = 0;

    for (;;) {
        acresToBuy = readIntNonNegative("How many acres do you wish to buy? ");
        double cost = static_cast<double>(acresToBuy) * s.landPriceThisYear;
        if (cost <= s.grainBushels + 1e-9) break;
        std::cout << "Not enough grain to buy that much land.\n";
    }

    if (acresToBuy == 0) {
        for (;;) {
            acresToSell = readIntNonNegative("How many acres do you wish to sell? ");
            if (acresToSell <= s.landAcres) break;
            std::cout << "You don't have that much land.\n";
        }
    }

    s.landAcres += acresToBuy;
    s.landAcres -= acresToSell;

    s.grainBushels -= static_cast<double>(acresToBuy) * s.landPriceThisYear;
    s.grainBushels += static_cast<double>(acresToSell) * s.landPriceThisYear;

    // 2) Выделение зерна.
    int bushelsToFeed = 0;
    for (;;) {
        bushelsToFeed = readIntNonNegative("How many bushels of grain do you wish to feed the people? ");
        if (bushelsToFeed <= s.grainBushels + 1e-9) break;
        std::cout << "You have only " << formatGrain(s.grainBushels) << " bushels in storage.\n";
    }
    s.grainBushels -= bushelsToFeed;

    // 3) Посадка.
    int acresToPlant = 0;
    for (;;) {
        acresToPlant = readIntNonNegative("How many acres do you wish to plant? ");
        if (acresToPlant > s.landAcres) {
            std::cout << "You have only " << s.landAcres << " acres.\n";
            continue;
        }
        if (acresToPlant > s.population * r.acresPerPersonMax) {
            std::cout << "Your people can work at most " << (s.population * r.acresPerPersonMax)
                      << " acres.\n";
            continue;
        }
        double seedsNeeded = acresToPlant * r.seedsBushelsPerAcre;
        if (seedsNeeded > s.grainBushels + 1e-9) {
            std::cout << "Not enough grain for seed. Needed " << formatGrain(seedsNeeded)
                      << ", in storage " << formatGrain(s.grainBushels) << ".\n";
            continue;
        }
        s.grainBushels -= seedsNeeded;
        break;
    }

    s.awaitingPlayerDecisions = false;

    // Посевы
    const int yieldPerAcre = rng.intInRange(r.yieldPerAcreMin, r.yieldPerAcreMax);
    const int harvestTotal = acresToPlant * yieldPerAcre;
    s.grainBushels += harvestTotal;

    // Крысы
    int maxRats = static_cast<int>(std::floor(s.grainBushels * r.ratsMaxFraction));
    if (maxRats < 0) maxRats = 0;
    int ratsAte = (maxRats == 0) ? 0 : rng.intInRange(0, maxRats);
    s.grainBushels -= ratsAte;
    if (s.grainBushels < 0.0) s.grainBushels = 0.0;

    // Голов
    const int populationStart = s.population;
    const int peopleFed = bushelsToFeed / r.bushelsPerPersonPerYear;
    const int starved = std::max(0, populationStart - peopleFed);

    const double starvedPercent = (populationStart == 0)
        ? 0.0
        : (100.0 * static_cast<double>(starved) / static_cast<double>(populationStart));

    if (starvedPercent > r.starvationLossFraction * 100.0 + 1e-9) {
        s.starvedLastYear = starved;
        s.immigrantsLastYear = 0;
        s.plagueLastYear = false;
        s.yieldPerAcreLastYear = yieldPerAcre;
        s.harvestTotalLastYear = harvestTotal;
        s.ratsAteLastYear = ratsAte;

        std::cout << "\nMore than " << static_cast<int>(r.starvationLossFraction * 100)
                  << "% of the population starved. You have been overthrown.\n";
        return false;
    }

    s.population -= starved;
    if (s.population <= 0) {
        std::cout << "\nAll people have died. Game over.\n";
        return false;
    }

    // Иммиграция
    int immigrants = static_cast<int>(
        (starved / 2.0) + (5.0 - static_cast<double>(yieldPerAcre)) * (s.grainBushels / 600.0) + 1.0
    );
    immigrants = std::clamp(immigrants, r.immigrantsMin, r.immigrantsMax);
    s.population += immigrants;

    // Чума
    bool plague = rng.chance(r.plagueProbability);
    if (plague) {
        s.population /= 2; // round down
        if (s.population <= 0) {
            std::cout << "\nThe plague wiped everyone out. Game over.\n";
            return false;
        }
    }

    s.starvedLastYear = starved;
    s.immigrantsLastYear = immigrants;
    s.plagueLastYear = plague;
    s.yieldPerAcreLastYear = yieldPerAcre;
    s.harvestTotalLastYear = harvestTotal;
    s.ratsAteLastYear = ratsAte;

    s.yearsCompleted += 1;
    s.starvationPercentSum += starvedPercent;

    s.year += 1;
    return true;
}

void printFinalScore(const GameState& s, const GameRules& r) {
    std::cout << "\n========================================\n";
    std::cout << "Summary of your rule\n";
    std::cout << "----------------------------------------\n";

    const double avgStarvedPercent = (s.yearsCompleted > 0)
        ? (s.starvationPercentSum / static_cast<double>(s.yearsCompleted))
        : 0.0;
    const double acresPerCitizen = (s.population > 0)
        ? (static_cast<double>(s.landAcres) / static_cast<double>(s.population))
        : 0.0;

    std::cout << "Average percent starved per year (P): " << avgStarvedPercent << "%\n";
    std::cout << "Acres of land per citizen (L): " << acresPerCitizen << "\n\n";

    if (avgStarvedPercent > r.pBadLower && acresPerCitizen < r.lBadUpper) {
        std::cout << "Terrible: you were driven out of the city.\n";
    } else if (avgStarvedPercent > r.pOkLower && acresPerCitizen < r.lOkUpper) {
        std::cout << "Mediocre: your rule was harsh, but the city survived.\n";
    } else if (avgStarvedPercent > r.pGoodLower && acresPerCitizen < r.lGoodUpper) {
        std::cout << "Good: you managed the city fairly well.\n";
    } else {
        std::cout << "Excellent: outstanding rule!\n";
    }
}

} 

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    GameRules rules;
    if (!rules.loadFromFile("game_rules.txt")) {
        std::cerr << "Failed to read game_rules.txt or the file contains errors.\n";
        return 1;
    }

    SaveManager saves("game_save.txt");
    GameState state;
    bool loaded = false;

    if (saves.hasValidSave()) {
        bool cont = askYesNo("A saved game was found. Continue? (Y/N): ");
        if (cont) {
            state = saves.load();
            loaded = IsValidSave(state);
            if (!loaded) {
                std::cout << "The save file is corrupted. Starting a new game.\n";
            }
        }
    }

    if (!loaded) {
        initNewGame(state, rules);
        Rng rng;
        state.landPriceThisYear = rng.intInRange(rules.landPriceMin, rules.landPriceMax);
        state.awaitingPlayerDecisions = true;
        saves.save(state);
    }

    Rng rng;

    while (state.year <= rules.totalYears) {
        bool ok = playOneYear(state, rules, rng, saves);

        if (!ok) {
            if (!state.awaitingPlayerDecisions) {
                saves.clear();
            }
            return 0;
        }

        saves.save(state);
    }

    printFinalScore(state, rules);
    saves.clear(); 
    return 0;
}
