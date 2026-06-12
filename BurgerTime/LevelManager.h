#pragma once
#include "LevelData.h"
#include "Singleton.h"
#include "IEventListener.h"
#include "GameMode.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <map>

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
        void LoadLevel(int levelId, dae::Scene& scene);
        void UnloadCurrentLevel(dae::Scene& scene);

        const LevelData* GetCurrentLevelData() const { return m_CurrentLevelData.get(); }
        int GetCurrentLevelId() const { return m_CurrentLevelId; }

        bool IsOnPlatform(float x, float y) const;
        bool IsOnLadder(float x, float y) const;
        bool IsPointOnPlatform(float x, float feetY) const;

        Player* GetPlayer1() const { return m_pPlayer1; }
        Player* GetPlayer2() const { return m_pPlayer2; }
        dae::GameObject* GetPlayer1Object() const;

        bool IsLevelComplete() const;
        float GetNearestPlatformY(float x, float y) const;
        float GetNearestLadderX(float x, float y) const;
        void RemoveEnemy(Enemy* enemy);

        static glm::vec2 GridToScreen(const glm::ivec2& gridPos);
        static glm::vec2 GridToScreen(int gridX, int gridY);

        const std::vector<BurgerPiece*>& GetBurgerPieces() const { return m_BurgerPieces; }
        const std::vector<Enemy*>& GetEnemies() const { return m_Enemies; }

        struct PlatSection
        {
            int   id;
            float surfaceY;
            float xMin;
            float xMax;
        };

        struct NavEdge
        {
            float ladderX;
            int   destSectionId;
        };

        void BuildNavGraph();
        std::vector<NavEdge> FindEnemyPath(int fromSection, int toSection) const;
        int GetEntitySection(float x, float y) const;
        const PlatSection* GetSection(int id) const;

        void SpawnPepperCloud(float x, float y);
        void OnEvent(const dae::Event& event) override;

        dae::GameObject* GetPlayer2Object()  const;
        dae::GameObject* GetPlayerDogObject() const;
        int GetNearestSection(float x, float y) const;
        int GetNearestSectionExcluding(float x, float y, int excludeId) const;
        void SetGameMode(GameMode mode) { m_GameMode = mode; }
        GameMode GetGameMode() const { return m_GameMode; }

        void ClearPlayerDog() { m_pPlayerDog = nullptr; }

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
        dae::GameObject* CreatePlayer(dae::Scene& scene, int playerId);

        std::vector<PlatSection> m_Sections;
        std::map<int, std::vector<NavEdge>> m_NavGraph;

        dae::Scene* m_pScene{ nullptr };

        void OnLevelComplete();
        bool m_LevelComplete{ false };

        dae::GameObject* CreatePlayer2(dae::Scene& scene);
        dae::GameObject* CreatePlayerDog(dae::Scene& scene);

        Player* m_pPlayer2{ nullptr };
        PlayerDog* m_pPlayerDog{ nullptr };
        GameMode   m_GameMode{ GameMode::SinglePlayer };
    };
}