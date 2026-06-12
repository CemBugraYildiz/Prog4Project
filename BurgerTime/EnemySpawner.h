#pragma once
#include "Component.h"
#include "Enemy.h"
#include "GameMode.h"
#include <vector>
#include <glm/glm.hpp>

namespace dae { class Scene; }

namespace BurgerTime
{
    class EnemySpawner final : public dae::Component
    {
    public:
        EnemySpawner(dae::GameObject* owner, dae::Scene& scene,
            GameMode mode, std::vector<glm::vec2> spawnPoints);

        void Update() override;
        void InitialSpawn();
        void OnEnemyKilled();

        static constexpr float RESPAWN_DELAY = 3.0f;
        static constexpr float MIN_PLAYER_DIST = 128.0f;
        static constexpr int   MAX_ENEMIES_SP = 4;
        static constexpr int   MAX_ENEMIES_COOP = 5;

    private:
        void SpawnEnemyAt(int idx);
        int FindValidSpawnPoint() const;
        bool IsPlayerFarFrom(const glm::vec2& pos) const;
        static EnemyType RandomEnemyType();

        dae::Scene& m_Scene;
        std::vector<glm::vec2> m_SpawnPoints;
        int m_MaxEnemies;

        struct PendingRespawn { float timer; };
        std::vector<PendingRespawn> m_Pending;
    };
}