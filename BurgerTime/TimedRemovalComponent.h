#pragma once
#include <Component.h>
#include <EngineTime.h>

namespace BurgerTime
{
    class TimedRemovalComponent final : public dae::Component
    {
    public:
        explicit TimedRemovalComponent(dae::GameObject* owner, float duration)
            : Component(owner), m_TimeLeft(duration) {
        }

        void Update() override
        {
            m_TimeLeft -= dae::EngineTime::GetDeltaTime();
            if (m_TimeLeft <= 0.0f)
                MarkForRemoval();
        }

    private:
        float m_TimeLeft;
    };
}
