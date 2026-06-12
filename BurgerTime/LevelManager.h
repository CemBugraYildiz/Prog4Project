#pragma once
#include "LevelData.h"
#include "Singleton.h"
#include "IEventListener.h"
#include "GameMode.h"
#include "NavGraph.h"
#include "CollisionWorld.h"
#include "EnemySpawner.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace dae { class Scene; class GameObject; }
namespace BurgerTime { class PlayerDog; }

namespace BurgerTime
{
    class Player;
    class BurgerPiece;
    class Enemy;

    class LevelManager final : public dae::Singleton<LevelManager>, public dae::IEventListener
    {
    public:
        // Type aliases 
        using PlatSection = NavGraph::PlatSection;
        using NavEdge = NavGraph::NavEdge;

        void LoadLevel(int levelId, dae::Scene& scene);
        void UnloadCurrentLevel(dae::Scene& scene);

        const LevelData* GetCurrentLevelData() const { return m_CurrentLevelData.get(); }
        int  GetCurrentLevelId() const { return m_CurrentLevelId; }

        static glm::vec2 GridToScreen(const glm::ivec2& gridPos);
        static glm::vec2 GridToScreen(int gridX, int gridY);

        // Players
        Player* GetPlayer1() const { return m_pPlayer1; }
        Player* GetPlayer2() const { return m_pPlayer2; }
        dae::GameObject* GetPlayer1Object() const;
        dae::GameObject* GetPlayer2Object() const;
        dae::GameObject* GetPlayerDogObject() const;
        void ClearPlayerDog() { m_pPlayerDog = nullptr; }

        // Enemies
        void RegisterEnemy(Enemy* enemy);
        void RemoveEnemy(Enemy* enemy);
        const std::vector<Enemy*>& GetEnemies() const { return m_Enemies; }

        // Burgers
        const std::vector<BurgerPiece*>& GetBurgerPieces() const { return m_BurgerPieces; }

        // Game state
        bool     IsLevelComplete() const;
        void     SetGameMode(GameMode mode) { m_GameMode = mode; }
        GameMode GetGameMode() const { return m_GameMode; }
        void     OnEvent(const dae::Event& event) override;
        void     SpawnPepperCloud(float x, float y);

        // Collision — delegates to CollisionWorld
        bool  IsOnPlatform(float x, float y) const { return m_Collision.IsOnPlatform(x, y); }
        bool  IsOnLadder(float x, float y) const { return m_Collision.IsOnLadder(x, y); }
        bool  IsPointOnPlatform(float x, float fy) const { return m_Collision.IsPointOnPlatform(x, fy); }
        float GetNearestPlatformY(float x, float y) const { return m_Collision.GetNearestPlatformY(x, y); }
        float GetNearestLadderX(float x, float y) const { return m_Collision.GetNearestLadderX(x, y); }

        // Navigation — delegates to NavGraph
        void BuildNavGraph() { m_Nav.Build(*m_CurrentLevelData); }
        std::vector<NavEdge> FindEnemyPath(int from, int to) const { return m_Nav.FindPath(from, to); }
        int GetEntitySection(float x, float y) const { return m_Nav.GetEntitySection(x, y); }
        const PlatSection* GetSection(int id) const { return m_Nav.GetSection(id); }
        int GetNearestSection(float x, float y) const { return m_Nav.GetNearestSection(x, y); }
        int GetNearestSectionExcluding(float x, float y, int ex) const { return m_Nav.GetNearestSectionExcluding(x, y, ex); }

    private:
        friend class dae::Singleton<LevelManager>;
        LevelManager() = default;

        int                        m_CurrentLevelId{ 0 };
        std::unique_ptr<LevelData> m_CurrentLevelData;
        dae::Scene* m_pScene{ nullptr };
        GameMode                   m_GameMode{ GameMode::SinglePlayer };
        bool                       m_LevelComplete{ false };

        Player* m_pPlayer1{ nullptr };
        Player* m_pPlayer2{ nullptr };
        PlayerDog* m_pPlayerDog{ nullptr };

        std::vector<BurgerPiece*> m_BurgerPieces;
        std::vector<Enemy*>       m_Enemies;
        EnemySpawner* m_pEnemySpawner{ nullptr };

        NavGraph       m_Nav;
        CollisionWorld m_Collision;

        // Creation helpers
        void CreatePlatforms(dae::Scene& scene);
        void CreateLadders(dae::Scene& scene);
        void CreatePlates(dae::Scene& scene);
        void CreateBurgers(dae::Scene& scene);
        void CreateEnemies(dae::Scene& scene);
        dae::GameObject* CreatePlayer(dae::Scene& scene, int playerId);
        dae::GameObject* CreatePlayer2(dae::Scene& scene);
        dae::GameObject* CreatePlayerDog(dae::Scene& scene);

        void OnLevelComplete();
    };
}