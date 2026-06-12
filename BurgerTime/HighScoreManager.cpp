#include "HighScoreManager.h"
#include <fstream>
#include <algorithm>

namespace BurgerTime
{
    void HighScoreManager::Load()
    {
        m_Entries.clear();
        std::ifstream f(FILE_PATH);
        if (!f) return;
        std::string name; int score;
        while (f >> name >> score)
            m_Entries.push_back({ name, score });
        std::sort(m_Entries.begin(), m_Entries.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b) { return a.score > b.score; });
        if ((int)m_Entries.size() > MAX_ENTRIES)
            m_Entries.resize(MAX_ENTRIES);
    }

    void HighScoreManager::Save() const
    {
        std::ofstream f(FILE_PATH);
        for (const auto& e : m_Entries)
            f << e.name << " " << e.score << "\n";
    }

    void HighScoreManager::Add(const std::string& name, int score)
    {
        m_Entries.push_back({ name, score });
        std::sort(m_Entries.begin(), m_Entries.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b) { return a.score > b.score; });
        if ((int)m_Entries.size() > MAX_ENTRIES)
            m_Entries.resize(MAX_ENTRIES);
        Save();
    }

    bool HighScoreManager::IsHighScore(int score) const
    {
        if ((int)m_Entries.size() < MAX_ENTRIES) return score > 0;
        return score > m_Entries.back().score;
    }
}