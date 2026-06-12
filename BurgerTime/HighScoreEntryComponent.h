#pragma once
#include <Component.h>
#include <string>

namespace dae { class TextComponent; }

namespace BurgerTime
{
    class HighScoreEntryComponent final : public dae::Component
    {
    public:
        HighScoreEntryComponent(dae::GameObject* owner, int score,
            dae::TextComponent* nameDisplay, dae::TextComponent* wheelDisplay);

        void Update() override {}

        void MoveLetter(int delta);
        void ConfirmLetter();
        void DeleteLetter();
        void RefreshDisplay();

    private:
        static constexpr int MAX_NAME = 3;
        static constexpr int WHEEL_LEN = 27; // A-Z(0-25) + END(26)

        std::string GetWheelString() const;

        int   m_Score;
        int   m_WheelIdx{ 0 };
        std::string m_Name;
        dae::TextComponent* m_NameTC{ nullptr };
        dae::TextComponent* m_WheelTC{ nullptr };
    };
}
