#include "EnemySpawner.h"
#include "LevelManager.h"
#include "GameObject.h"
#include "EngineTime.h"
#include "Scene.h"
#include "Player.h"
#include <random>
#include <algorithm>
#include <array>

namespace BurgerTime
{
    EnemySpawner::EnemySpawner(dae::GameObject* owner, dae::Scene& scene,
        GameMode mode, std::vector<glm::vec2> spawnPoints)
        : Component(owner)
        , m_Scene(scene)
        , m_SpawnPoints(std::move(spawnPoints))
        , m_MaxEnemies(mode == GameMode::CoOp ? MAX_ENEMIES_COOP : MAX_ENEMIES_SP)
    {
    }

    void EnemySpawner::InitialSpawn()
    {
        if (m_SpawnPoints.empty()) return;

        std::mt19937 rng{ std::random_device{}() };

        std::array<EnemyType, 3> types{
            EnemyType::Egg, EnemyType::Pickle, EnemyType::Sausage
        };
        std::shuffle(types.begin(), types.end(), rng);

        int guaranteed = std::min(static_cast<int>(m_SpawnPoints.size()), 3);
        for (int i = 0; i < guaranteed; ++i)
        {
            auto go = std::make_unique<dae::GameObject>();
            go->SetPosition(m_SpawnPoints[i].x, m_SpawnPoints[i].y);
            go->AddComponent<Enemy>(types[i]);
            m_Scene.Add(std::move(go));
        }

        std::uniform_int_distribution<int> idxDist(0, static_cast<int>(m_SpawnPoints.size()) - 1);
        int extras = m_MaxEnemies - guaranteed;
        for (int i = 0; i < extras; ++i)
        {
            auto go = std::make_unique<dae::GameObject>();
            int idx = idxDist(rng);
            go->SetPosition(m_SpawnPoints[idx].x, m_SpawnPoints[idx].y);
            go->AddComponent<Enemy>(RandomEnemyType());
            m_Scene.Add(std::move(go));
        }
    }

    void EnemySpawner::Update()
    {
        if (LevelManager::GetInstance().IsLevelComplete()) return;

        float dt = dae::EngineTime::GetDeltaTime();
        for (auto& p : m_Pending)
            if (p.timer > 0.f) p.timer -= dt;

        m_Pending.erase(
            std::remove_if(m_Pending.begin(), m_Pending.end(),
                [&](PendingRespawn& p)
                {
                    if (p.timer > 0.f) return false;
                    int idx = FindValidSpawnPoint();
                    if (idx < 0) return false;
                    SpawnEnemyAt(idx);
                    return true;
                }),
            m_Pending.end());
    }

    void EnemySpawner::OnEnemyKilled()
    {
        auto& lm = LevelManager::GetInstance();
        int active = static_cast<int>(lm.GetEnemies().size());
        int pending = static_cast<int>(m_Pending.size());
        if (active + pending < m_MaxEnemies)
            m_Pending.push_back({ RESPAWN_DELAY });
    }

    void EnemySpawner::SpawnEnemyAt(int idx)
    {
        if (idx < 0 || idx >= static_cast<int>(m_SpawnPoints.size())) return;
        auto go = std::make_unique<dae::GameObject>();
        go->SetPosition(m_SpawnPoints[idx].x, m_SpawnPoints[idx].y);
        go->AddComponent<Enemy>(RandomEnemyType());
        m_Scene.Add(std::move(go));
    }

    int EnemySpawner::FindValidSpawnPoint() const
    {
        std::vector<int> valid;
        for (int i = 0; i < static_cast<int>(m_SpawnPoints.size()); ++i)
            if (IsPlayerFarFrom(m_SpawnPoints[i]))
                valid.push_back(i);

        if (valid.empty()) return -1;

        std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(0, static_cast<int>(valid.size()) - 1);
        return valid[dist(rng)];
    }

    bool EnemySpawner::IsPlayerFarFrom(const glm::vec2& pos) const
    {
        auto& lm = LevelManager::GetInstance();
        auto Check = [&](Player* p) {
            if (!p || !p->IsAlive()) return true;
            return std::abs(p->GetPosition().x - pos.x) >= MIN_PLAYER_DIST;
            };
        return Check(lm.GetPlayer1()) && Check(lm.GetPlayer2());
    }

    EnemyType EnemySpawner::RandomEnemyType()
    {
        static std::mt19937 rng{ std::random_device{}() };
        static std::uniform_int_distribution<int> dist(0, 2);
        switch (dist(rng))
        {
        case 0:  return EnemyType::Egg;
        case 1:  return EnemyType::Pickle;
        default: return EnemyType::Sausage;
        }
    }
}