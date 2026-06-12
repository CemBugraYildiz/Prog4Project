#include "HighScoreEntryComponent.h"
#include "GameManager.h"
#include "HighScoreManager.h"
#include "TextComponent.h"

namespace BurgerTime
{
    static std::string SlotAt(int idx)
    {
        idx = ((idx % 27) + 27) % 27;
        if (idx < 26) return std::string(1, 'A' + idx);
        return "END";
    }

    HighScoreEntryComponent::HighScoreEntryComponent(dae::GameObject* owner, int score,
        dae::TextComponent* nameDisplay, dae::TextComponent* wheelDisplay)
        : Component(owner)
        , m_Score(score)
        , m_NameTC(nameDisplay)
        , m_WheelTC(wheelDisplay)
    {
    }

    void HighScoreEntryComponent::MoveLetter(int delta)
    {
        m_WheelIdx = ((m_WheelIdx + delta) % 27 + 27) % 27;
        RefreshDisplay();
    }

    void HighScoreEntryComponent::ConfirmLetter()
    {
        if (m_WheelIdx == 26) // END
        {
            if (m_Name.empty()) return;
            HighScoreManager::GetInstance().Add(m_Name, m_Score);
            GameManager::GetInstance().ShowHighScoreDisplay();
            return;
        }
        if ((int)m_Name.size() < MAX_NAME)
        {
            m_Name += ('A' + m_WheelIdx);
            if ((int)m_Name.size() == MAX_NAME)
            {
                HighScoreManager::GetInstance().Add(m_Name, m_Score);
                GameManager::GetInstance().ShowHighScoreDisplay();
                return;
            }
        }
        RefreshDisplay();
    }

    void HighScoreEntryComponent::DeleteLetter()
    {
        if (!m_Name.empty()) m_Name.pop_back();
        RefreshDisplay();
    }

    void HighScoreEntryComponent::RefreshDisplay()
    {
        // Name slots: "P _ _" style
        std::string nameStr;
        for (int i = 0; i < MAX_NAME; ++i)
        {
            if (i > 0) nameStr += "  ";
            nameStr += (i < (int)m_Name.size()) ? std::string(1, m_Name[i]) : "_";
        }
        if (m_NameTC) m_NameTC->SetText(nameStr);

        // Wheel: show 7 slots centered on current, highlight with [ ]
        if (m_WheelTC) m_WheelTC->SetText(GetWheelString());
    }

    std::string HighScoreEntryComponent::GetWheelString() const
    {
        std::string result;
        for (int d = -3; d <= 3; ++d)
        {
            std::string s = SlotAt(m_WheelIdx + d);
            if (d == 0)
                result += "[" + s + "]";
            else
                result += " " + s + " ";
        }
        return result;
    }
}