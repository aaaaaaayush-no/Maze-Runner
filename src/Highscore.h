#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

struct HighscoreEntry {
    std::string name;
    int score;
    float time;
    int difficulty;   // 0=EASY, 1=MEDIUM, 2=HARD, 3=NIGHTMARE
    int collectables;
    int stars;
    int perfectRun;
};

inline std::vector<HighscoreEntry> loadHighscores(const std::string& path) {
    std::vector<HighscoreEntry> entries;
    std::ifstream file(path);
    if (!file.is_open()) return entries;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ','))
            tokens.push_back(token);

        if (tokens.size() < 5) continue;

        HighscoreEntry e;
        e.name = tokens[0];
        e.score = std::stoi(tokens[1]);
        e.time = std::stof(tokens[2]);
        e.difficulty = std::stoi(tokens[3]);
        e.collectables = std::stoi(tokens[4]);

        // Backward compatibility: old files may have 5 columns
        if (tokens.size() >= 7) {
            e.stars = std::stoi(tokens[5]);
            e.perfectRun = std::stoi(tokens[6]);
        } else {
            e.stars = 1;
            e.perfectRun = 0;
        }

        entries.push_back(e);
    }
    return entries;
}

inline void saveHighscores(const std::string& path,
                           const std::vector<HighscoreEntry>& entries) {
    std::ofstream file(path);
    if (!file.is_open()) return;

    for (auto& e : entries) {
        file << e.name << ","
             << e.score << ","
             << e.time << ","
             << e.difficulty << ","
             << e.collectables << ","
             << e.stars << ","
             << e.perfectRun << "\n";
    }
}

inline void addHighscore(const std::string& path, const HighscoreEntry& entry) {
    auto entries = loadHighscores(path);
    entries.push_back(entry);
    // Sort by score descending
    std::sort(entries.begin(), entries.end(),
              [](const HighscoreEntry& a, const HighscoreEntry& b) {
                  return a.score > b.score;
              });
    // Keep top 10
    if (entries.size() > 10)
        entries.resize(10);
    saveHighscores(path, entries);
}

// Get best stars per difficulty (for title screen preview)
inline int getBestStars(const std::vector<HighscoreEntry>& entries, int difficulty) {
    int best = 0;
    for (auto& e : entries) {
        if (e.difficulty == difficulty && e.stars > best)
            best = e.stars;
    }
    return best;
}

#endif
