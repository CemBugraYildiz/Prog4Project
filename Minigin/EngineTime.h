#pragma once

namespace dae
{
    class EngineTime
    {
    public:
        static float GetDeltaTime() noexcept { return s_DeltaTime; }
        static void SetDeltaTime(float dt) noexcept { s_DeltaTime = dt; }

    private:
        static inline float s_DeltaTime{ 0.016f };
    };
}
