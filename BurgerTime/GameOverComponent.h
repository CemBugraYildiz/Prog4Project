#pragma once
#include <Component.h>

namespace BurgerTime
{
    class GameOverComponent final : public dae::Component
    {
    public:
        GameOverComponent(dae::GameObject* owner, int score);
        void Update() override;
        void Confirm();

    private:
        int   m_Score;
        float m_AutoTimer{ 0.f };
        bool  m_Triggered{ false };
        static constexpr float AUTO_ADVANCE = 8.f;
    };
}
