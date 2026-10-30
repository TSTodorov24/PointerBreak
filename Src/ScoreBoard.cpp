// ScoreBoard.cpp - high score tables with validation of everything read from the file.
#include "ScoreBoard.h"

#include <fstream>
#include <sstream>

bool ScoreBoard::IsValidName(const std::string& name) {
    if (name.size() != 3) {
        return false;
    }
    for (char letter : name) {
        if (letter < 'A' || letter > 'Z') {
            return false;
        }
    }
    return true;
}

void ScoreBoard::LoadFromFile(const std::string& path) {
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        m_entries[i].clear();
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return;   // no file yet: start with empty tables
    }

    // Every line: difficulty name score hintUsed   (example: 1 ABC 850 0)
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        int difficulty = 0;
        int hint = 0;
        ScoreEntry entry;
        if (!(lineStream >> difficulty >> entry.name >> entry.score >> hint)) {
            continue;   // broken line
        }
        if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT || !IsValidName(entry.name) ||
            entry.score < 0 || entry.score > MAX_SCORE || (hint != 0 && hint != 1)) {
            continue;   // values out of range
        }
        entry.hintUsed = (hint == 1);
        AddScore(difficulty, entry);
    }
}

void ScoreBoard::SaveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return;   // cannot save, the game keeps working
    }
    for (int d = 0; d < DIFFICULTY_COUNT; d++) {
        for (const ScoreEntry& entry : m_entries[d]) {
            file << d << " " << entry.name << " " << entry.score << " " << (entry.hintUsed ? 1 : 0) << "\n";
        }
    }
}

bool ScoreBoard::QualifiesForTable(int difficulty, int score) const {
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT || score <= 0) {
        return false;
    }
    const std::vector<ScoreEntry>& list = m_entries[difficulty];
    if (static_cast<int>(list.size()) < MAX_SCORES_PER_TABLE) {
        return true;
    }
    return score > list.back().score;
}

void ScoreBoard::AddScore(int difficulty, const ScoreEntry& entry) {
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT) {
        return;
    }
    std::vector<ScoreEntry>& list = m_entries[difficulty];

    // Insert so that the list stays sorted from the best score to the worst.
    size_t position = 0;
    while (position < list.size() && list[position].score >= entry.score) {
        position++;
    }
    list.insert(list.begin() + static_cast<long>(position), entry);

    while (static_cast<int>(list.size()) > MAX_SCORES_PER_TABLE) {
        list.pop_back();
    }
}

const std::vector<ScoreEntry>& ScoreBoard::GetEntries(int difficulty) const {
    static const std::vector<ScoreEntry> empty;
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT) {
        return empty;
    }
    return m_entries[difficulty];
}
