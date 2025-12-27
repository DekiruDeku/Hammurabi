#include "SaveManager.hpp"

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "GameState.hpp"

SaveManager::SaveManager(std::string path) : filePath(std::move(path)) {}

static void writeKV(std::ofstream& out, const std::string& k, const std::string& v) {
    out << k << '=' << v << '\n';
}

static std::string trimCopy(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r')) ++b;
    size_t e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r')) --e;
    return s.substr(b, e - b);
}

bool SaveManager::save(const GameState& state) const {
    std::ofstream out(filePath, std::ios::trunc);
    if (!out) return false;

    out << std::setprecision(17);

    writeKV(out, "year", std::to_string(state.year));
    writeKV(out, "population", std::to_string(state.population));
    writeKV(out, "grain", std::to_string(state.grainBushels));
    writeKV(out, "land", std::to_string(state.landAcres));

    writeKV(out, "starvedLastYear", std::to_string(state.starvedLastYear));
    writeKV(out, "immigrantsLastYear", std::to_string(state.immigrantsLastYear));
    writeKV(out, "plagueLastYear", state.plagueLastYear ? "1" : "0");
    writeKV(out, "yieldPerAcreLastYear", std::to_string(state.yieldPerAcreLastYear));
    writeKV(out, "harvestTotalLastYear", std::to_string(state.harvestTotalLastYear));
    writeKV(out, "ratsAteLastYear", std::to_string(state.ratsAteLastYear));

    writeKV(out, "landPriceThisYear", std::to_string(state.landPriceThisYear));
    writeKV(out, "awaitingPlayerDecisions", state.awaitingPlayerDecisions ? "1" : "0");

    writeKV(out, "yearsCompleted", std::to_string(state.yearsCompleted));
    writeKV(out, "starvationPercentSum", std::to_string(state.starvationPercentSum));

    return true;
}

GameState SaveManager::load() const {
    GameState state;

    std::ifstream in(filePath);
    if (!in) {
        state.year = -1;
        return state;
    }

    try {
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string key, value;
            if (!std::getline(ss, key, '=')) continue;
            if (!std::getline(ss, value)) continue;
            key = trimCopy(key);
            value = trimCopy(value);

            if (key == "year") state.year = std::stoi(value);
            else if (key == "population") state.population = std::stoi(value);
            else if (key == "grain") state.grainBushels = std::stod(value);
            else if (key == "land") state.landAcres = std::stoi(value);

            else if (key == "starvedLastYear") state.starvedLastYear = std::stoi(value);
            else if (key == "immigrantsLastYear") state.immigrantsLastYear = std::stoi(value);
            else if (key == "plagueLastYear") state.plagueLastYear = (value == "1");
            else if (key == "yieldPerAcreLastYear") state.yieldPerAcreLastYear = std::stoi(value);
            else if (key == "harvestTotalLastYear") state.harvestTotalLastYear = std::stoi(value);
            else if (key == "ratsAteLastYear") state.ratsAteLastYear = std::stoi(value);

            else if (key == "landPriceThisYear") state.landPriceThisYear = std::stoi(value);
            else if (key == "awaitingPlayerDecisions") state.awaitingPlayerDecisions = (value == "1");

            else if (key == "yearsCompleted") state.yearsCompleted = std::stoi(value);
            else if (key == "starvationPercentSum") state.starvationPercentSum = std::stod(value);
        }
    } catch (...) {
        state.year = -1;
        return state;
    }

    if (!IsValidSave(state)) {
        state.year = -1;
    }

    return state;
}

bool SaveManager::hasValidSave() const {
    GameState s = load();
    return IsValidSave(s);
}

bool SaveManager::clear() const {
    return std::remove(filePath.c_str()) == 0;
}
