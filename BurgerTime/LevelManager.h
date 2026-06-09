#pragma once
#include "LevelData.h"
#include "Singleton.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace dae { class Scene; class GameObject; }

namespace BurgerTime
{
    class Player;
    class BurgerPiece;
    class Enemy;

    class LevelManager final : public dae::Singleton<LevelManager>
    {
    public:
        void LoadLevel(int levelId, dae::Scene& scene);
        void UnloadCurrentLevel(dae::Scene& scene);

        const LevelData* GetCurrentLevelData() const { return m_CurrentLevelData.get(); }
        int GetCurrentLevelId() const { return m_CurrentLevelId; }

        bool IsOnPlatform(float x, float y) const;
        bool IsOnLadder(float x, float y) const;
        bool IsPointOnPlatform(float x, float feetY) const;

        Player* GetPlayer1() const { return m_pPlayer1; }
        dae::GameObject* GetPlayer1Object() const;

        bool IsLevelComplete() const;
        float GetNearestPlatformY(float x, float y) const;
        float GetNearestLadderX(float x, float y) const;

        static glm::vec2 GridToScreen(const glm::ivec2& gridPos);
        static glm::vec2 GridToScreen(int gridX, int gridY);

        const std::vector<BurgerPiece*>& GetBurgerPieces() const { return m_BurgerPieces; }

    private:
        friend class dae::Singleton<LevelManager>;
        LevelManager() = default;

        int m_CurrentLevelId{ 0 };
        std::unique_ptr<LevelData> m_CurrentLevelData;

        Player* m_pPlayer1{ nullptr };
        std::vector<BurgerPiece*> m_BurgerPieces;   
        std::vector<Enemy*>       m_Enemies;          

        void CreatePlatforms(dae::Scene& scene);
        void CreateLadders(dae::Scene& scene);
        void CreatePlates(dae::Scene& scene);
        void CreateBurgers(dae::Scene& scene);
        void CreateEnemies(dae::Scene& scene);
        void CreatePepperPickups(dae::Scene& scene);
        dae::GameObject* CreatePlayer(dae::Scene& scene, int playerId);
    };
}