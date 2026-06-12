#include "CollisionWorld.h"
#include "LevelData.h"
#include "LevelManager.h"
#include "GameConfig.h"
#include <cmath>
#include <limits>

namespace BurgerTime
{
    bool CollisionWorld::IsOnPlatform(float x, float y) const
    {
        if (!m_Data) return false;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;

        for (const auto& p : m_Data->platforms)
        {
            auto sp = LevelManager::GridToScreen(p);
            if (x >= sp.x - tol && x <= sp.x + tileSize + tol &&
                std::abs(feetY - sp.y) < tol)
                return true;
        }
        return false;
    }

    bool CollisionWorld::IsOnLadder(float x, float y) const
    {
        if (!m_Data) return false;
        const float xTol = Config::LADDER_X_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;

        for (const auto& l : m_Data->ladders)
        {
            auto sp = LevelManager::GridToScreen(l);
            if (feetY < sp.y || feetY > sp.y + tileSize) continue;
            if (x + Config::PLAYER_WIDTH <= sp.x - xTol) continue;
            if (x >= sp.x + tileSize + xTol) continue;
            return true;
        }
        return false;
    }

    bool CollisionWorld::IsPointOnPlatform(float x, float feetY) const
    {
        if (!m_Data) return false;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);

        for (const auto& p : m_Data->platforms)
        {
            auto sp = LevelManager::GridToScreen(p);
            if (x >= sp.x - tol && x <= sp.x + tileSize + tol &&
                std::abs(feetY - sp.y) < tol)
                return true;
        }
        return false;
    }

    float CollisionWorld::GetNearestPlatformY(float x, float y) const
    {
        if (!m_Data) return -1.f;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;
        float bestDist = tol + 1.f, bestY = -1.f;

        for (const auto& p : m_Data->platforms)
        {
            auto sp = LevelManager::GridToScreen(p);
            if (x < sp.x - tol || x > sp.x + tileSize + tol) continue;
            float dist = std::abs(feetY - sp.y);
            if (dist < tol && dist < bestDist) { bestDist = dist; bestY = sp.y; }
        }
        return bestY < 0.f ? -1.f : bestY - Config::PLAYER_HEIGHT;
    }

    float CollisionWorld::GetNearestLadderX(float x, float y) const
    {
        if (!m_Data) return -1.f;
        const float xTol = Config::LADDER_X_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;
        float bestDist = 9999.f, bestX = -1.f;

        for (const auto& l : m_Data->ladders)
        {
            auto sp = LevelManager::GridToScreen(l);
            if (feetY < sp.y || feetY > sp.y + tileSize)        continue;
            if (x + Config::PLAYER_WIDTH <= sp.x - xTol)         continue;
            if (x >= sp.x + tileSize + xTol)                     continue;

            float snapX = sp.x + (tileSize - Config::PLAYER_WIDTH) * 0.5f;
            float dist = std::abs(x - snapX);
            if (dist < bestDist) { bestDist = dist; bestX = snapX; }
        }
        return bestX;
    }
}