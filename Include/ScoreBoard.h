// ScoreBoard.h - top 5 scores for every difficulty, saved in a text file.
// Uses file streams only (no cin/cout).
#pragma once

#include <string>
#include <vector>

#include "Constants.h"

struct ScoreEntry {
    std::string name;   // exactly 3 letters A-Z
    int score;
    bool hintUsed;
};

class ScoreBoard {
public:
    void LoadFromFile(const std::string& path);      // bad lines are skipped, missing file = empty table
    void SaveToFile(const std::string& path) const;

    bool QualifiesForTable(int difficulty, int score) const;
    void AddScore(int difficulty, const ScoreEntry& entry);
    const std::vector<ScoreEntry>& GetEntries(int difficulty) const;

    static bool IsValidName(const std::string& name);

private:
    std::vector<ScoreEntry> m_entries[DIFFICULTY_COUNT];
};
