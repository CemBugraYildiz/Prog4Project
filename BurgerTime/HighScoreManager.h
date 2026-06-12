#pragma once
#include "Singleton.h"
#include <vector>
#include <string>

namespace BurgerTime
{
    struct HighScoreEntry { std::string name; int score{ 0 }; };

    class HighScoreManager final : public dae::Singleton<HighScoreManager>
    {
    public:
        void Load();
        void Save() const;
        void Add(const std::string& name, int score);
        bool IsHighScore(int score) const;
        const std::vector<HighScoreEntry>& GetEntries() const { return m_Entries; }

    private:
        friend class dae::Singleton<HighScoreManager>;
        HighScoreManager() = default;
        static constexpr int MAX_ENTRIES = 10;
        static constexpr const char* FILE_PATH = "Data/BurgerTime/highscores.dat";
        std::vector<HighScoreEntry> m_Entries;
    };
}
