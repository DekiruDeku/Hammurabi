#pragma once

#include <string>

struct GameState;

class SaveManager {
public:
    explicit SaveManager(std::string path = "game_save.txt");

    bool hasValidSave() const;
    bool save(const GameState& state) const;
    GameState load() const;
    bool clear() const;

private:
    std::string filePath;
};
