#pragma once

namespace BurgerTime
{
    struct LevelData;

    class CollisionWorld
    {
    public:
        void SetData(const LevelData* data) { m_Data = data; }

        bool  IsOnPlatform(float x, float y) const;
        bool  IsOnLadder(float x, float y) const;
        bool  IsPointOnPlatform(float x, float feetY) const;
        float GetNearestPlatformY(float x, float y) const;
        float GetNearestLadderX(float x, float y) const;

    private:
        const LevelData* m_Data{ nullptr };
    };
}
