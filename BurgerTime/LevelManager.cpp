#include "LevelManager.h"
#include "LevelLoader.h"
#include "GameConfig.h"
#include "Scene.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "SoundIds.h"
#include "Player.h"
#include "PlayerState.h"
#include "Enemy.h"
#include "AnimationComponent.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "RenderComponent.h"
#include "BurgerPiece.h"
#include "PepperCloud.h"
#include "EventQueue.h"
#include "PlayerDog.h"
#include "GameMode.h"
#include <algorithm>
#include <iostream>

namespace BurgerTime
{
    //  Helper: ingredient spawning (collapses the 6 burger loops)
    static void SpawnIngredients(const std::vector<glm::ivec2>& positions,
        IngredientType type,
        dae::Scene& scene,
        std::vector<BurgerPiece*>& out)
    {
        for (const auto& pos : positions)
        {
            auto sp = LevelManager::GridToScreen(pos);
            auto go = std::make_unique<dae::GameObject>();
            go->SetPosition(sp.x, sp.y);
            go->AddComponent<BurgerPiece>(type);
            if (auto* p = go->GetComponent<BurgerPiece>()) out.push_back(p);
            scene.Add(std::move(go));
        }
    }

    //  Grid - Screen
    glm::vec2 LevelManager::GridToScreen(int gridX, int gridY)
    {
        return glm::vec2{
            static_cast<float>(gridX) + Config::LEVEL_OFFSET_X,
            static_cast<float>(gridY) + Config::LEVEL_OFFSET_Y
        };
    }

    glm::vec2 LevelManager::GridToScreen(const glm::ivec2& gridPos)
    {
        return GridToScreen(gridPos.x, gridPos.y);
    }

    //  Level lifecycle
    void LevelManager::LoadLevel(int levelId, dae::Scene& scene)
    {
        m_pScene = &scene;
        UnloadCurrentLevel(scene);

        std::string filepath = "Data/BurgerTime/Levels/level"
            + std::to_string(levelId) + ".json";
        m_CurrentLevelData = LevelLoader::LoadFromFile(filepath, levelId);

        if (!m_CurrentLevelData)
        {
            std::cerr << "Failed to load level " << levelId << "\n";
            return;
        }

        m_CurrentLevelId = levelId;
        m_Collision.SetData(m_CurrentLevelData.get());

        CreatePlatforms(scene);
        CreateLadders(scene);
        CreatePlates(scene);
        CreateBurgers(scene);

        CreatePlayer(scene, 0);
        if (m_GameMode == GameMode::CoOp)   CreatePlayer2(scene);
        else if (m_GameMode == GameMode::Versus)  CreatePlayerDog(scene);

        BuildNavGraph();
        CreateEnemies(scene);

        m_LevelComplete = false;
        dae::EventQueue::GetInstance().AddListener(dae::EventType::LevelCompleted, this);
    }

    void LevelManager::UnloadCurrentLevel(dae::Scene& scene)
    {
        m_pScene = &scene;
        if (m_CurrentLevelId == 0) return;

        scene.Clear();
        m_CurrentLevelData.reset();
        m_CurrentLevelId = 0;
        m_pPlayer1 = nullptr;
        m_pPlayer2 = nullptr;
        m_pPlayerDog = nullptr;
        m_pEnemySpawner = nullptr;
        m_BurgerPieces.clear();
        m_Enemies.clear();

        m_Nav.Clear();
        m_Collision.SetData(nullptr);

        dae::EventQueue::GetInstance().RemoveListener(dae::EventType::LevelCompleted, this);
    }

    //  Actor creation
    dae::GameObject* LevelManager::CreatePlayer(dae::Scene& scene, int playerId)
    {
        if (!m_CurrentLevelData) return nullptr;

        glm::vec2 sp = GridToScreen(m_CurrentLevelData->peterSpawn);
        sp.y -= Config::PLAYER_HEIGHT;

        auto go = std::make_unique<dae::GameObject>();
        go->SetPosition(sp.x, sp.y);
        go->AddComponent<AnimationComponent>();
        go->AddComponent<Player>(playerId);
        go->AddComponent<dae::HealthComponent>(playerId, 1, 4);
        go->AddComponent<dae::ScoreComponent>(playerId);

        m_pPlayer1 = go->GetComponent<Player>();
        auto* ptr = go.get();
        scene.Add(std::move(go));
        return ptr;
    }

    dae::GameObject* LevelManager::CreatePlayer2(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return nullptr;

        glm::vec2 sp = GridToScreen(m_CurrentLevelData->peterJrSpawn);
        sp.y -= Config::PLAYER_HEIGHT;

        auto go = std::make_unique<dae::GameObject>();
        go->SetPosition(sp.x, sp.y);
        go->AddComponent<AnimationComponent>();
        go->AddComponent<Player>(1);
        go->AddComponent<dae::HealthComponent>(1, 1, 4);
        go->AddComponent<dae::ScoreComponent>(1);

        m_pPlayer2 = go->GetComponent<Player>();
        auto* ptr = go.get();
        scene.Add(std::move(go));
        return ptr;
    }

    dae::GameObject* LevelManager::CreatePlayerDog(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return nullptr;

        glm::vec2 sp = GridToScreen(m_CurrentLevelData->playerDogSpawn);
        sp.y -= Config::ENEMY_HEIGHT;

        auto go = std::make_unique<dae::GameObject>();
        go->SetPosition(sp.x, sp.y);
        go->AddComponent<AnimationComponent>();
        go->AddComponent<PlayerDog>();

        m_pPlayerDog = go->GetComponent<PlayerDog>();
        if (m_pPlayer1) m_pPlayerDog->SetPlayerTarget(m_pPlayer1);

        auto* ptr = go.get();
        scene.Add(std::move(go));
        return ptr;
    }

    dae::GameObject* LevelManager::GetPlayer1Object() const
    {
        return m_pPlayer1 ? m_pPlayer1->GetOwner() : nullptr;
    }

    dae::GameObject* LevelManager::GetPlayer2Object() const
    {
        return m_pPlayer2 ? m_pPlayer2->GetOwner() : nullptr;
    }

    dae::GameObject* LevelManager::GetPlayerDogObject() const
    {
        return m_pPlayerDog ? m_pPlayerDog->GetOwner() : nullptr;
    }

    //  World creation
    void LevelManager::CreatePlatforms(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;
        for (const auto& pos : m_CurrentLevelData->platforms)
        {
            auto sp = GridToScreen(pos);
            auto go = std::make_unique<dae::GameObject>();
            go->SetPosition(sp.x, sp.y);
            auto* r = go->AddComponent<dae::RenderComponent>("BurgerTime/World/platform.png");
            r->SetCustomSize(Config::PLATFORM_WIDTH, Config::PLATFORM_HEIGHT);
            scene.Add(std::move(go));
        }
    }

    void LevelManager::CreateLadders(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;
        for (const auto& pos : m_CurrentLevelData->ladders)
        {
            auto sp = GridToScreen(pos);
            auto go = std::make_unique<dae::GameObject>();
            go->SetPosition(sp.x, sp.y);
            auto* r = go->AddComponent<dae::RenderComponent>("BurgerTime/World/ladder.png");
            r->SetCustomSize(Config::LADDER_WIDTH, Config::LADDER_HEIGHT);
            scene.Add(std::move(go));
        }
    }

    void LevelManager::CreatePlates(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;
        for (const auto& pos : m_CurrentLevelData->plates)
        {
            auto sp = GridToScreen(pos);
            auto go = std::make_unique<dae::GameObject>();
            go->SetPosition(sp.x, sp.y);
            auto* r = go->AddComponent<dae::RenderComponent>("BurgerTime/World/plate.png");
            r->SetCustomSize(Config::PLATE_WIDTH, Config::PLATE_HEIGHT);
            scene.Add(std::move(go));
        }
    }

    void LevelManager::CreateBurgers(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;
        const auto& d = *m_CurrentLevelData;
        SpawnIngredients(d.topBuns, IngredientType::TOP_BUN, scene, m_BurgerPieces);
        SpawnIngredients(d.lettuces, IngredientType::LETTUCE, scene, m_BurgerPieces);
        SpawnIngredients(d.patties, IngredientType::PATTY, scene, m_BurgerPieces);
        SpawnIngredients(d.tomatoes, IngredientType::TOMATO, scene, m_BurgerPieces);
        SpawnIngredients(d.cheeses, IngredientType::CHEESE, scene, m_BurgerPieces);
        SpawnIngredients(d.bottomBuns, IngredientType::BOTTOM_BUN, scene, m_BurgerPieces);
    }

    void LevelManager::CreateEnemies(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;

        std::vector<glm::vec2> spawnPoints;
        for (const auto& p : m_CurrentLevelData->enemySpawns)
        {
            auto sp = GridToScreen(p);
            sp.y -= Config::ENEMY_HEIGHT;
            spawnPoints.push_back(sp);
        }

        auto spawnerGO = std::make_unique<dae::GameObject>();
        m_pEnemySpawner = spawnerGO->AddComponent<EnemySpawner>(
            scene, m_GameMode, std::move(spawnPoints));
        scene.Add(std::move(spawnerGO));

        if (m_pEnemySpawner)
            m_pEnemySpawner->InitialSpawn();
    }

    //  Enemy registry
    void LevelManager::RegisterEnemy(Enemy* enemy)
    {
        m_Enemies.push_back(enemy);
    }

    void LevelManager::RemoveEnemy(Enemy* enemy)
    {
        auto it = std::find(m_Enemies.begin(), m_Enemies.end(), enemy);
        if (it != m_Enemies.end())
            m_Enemies.erase(it);

        if (m_pEnemySpawner)
            m_pEnemySpawner->OnEnemyKilled();
    }

    //  Game state
    bool LevelManager::IsLevelComplete() const
    {
        if (m_BurgerPieces.empty()) return false;
        for (auto* piece : m_BurgerPieces)
            if (!piece->IsLanded()) return false;
        return true;
    }

    void LevelManager::SpawnPepperCloud(float x, float y)
    {
        if (!m_pScene) return;
        auto cloud = std::make_unique<dae::GameObject>();
        cloud->SetPosition(x, y);
        cloud->AddComponent<PepperCloudComponent>();
        m_pScene->Add(std::move(cloud));
    }

    void LevelManager::OnEvent(const dae::Event& event)
    {
        if (event.type == dae::EventType::LevelCompleted)
            OnLevelComplete();
    }

    void LevelManager::OnLevelComplete()
    {
        if (m_LevelComplete) return;
        m_LevelComplete = true;

        for (auto* enemy : m_Enemies)
            if (enemy) enemy->Freeze();

        if (m_pPlayerDog) m_pPlayerDog->Freeze();
        if (m_pPlayer1)   m_pPlayer1->TransitionToVictory();
        if (m_pPlayer2)   m_pPlayer2->TransitionToVictory();

        dae::ServiceLocator::GetSoundSystem().StopMusic();
        dae::ServiceLocator::GetSoundSystem().Play(dae::SOUND_WIN, 1.0f);
    }
}