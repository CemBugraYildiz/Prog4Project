#include "LevelManager.h"
#include "LevelLoader.h"
#include "GameConfig.h"
#include "Scene.h"
#include "GameObject.h"
#include "Player.h"
#include "Enemy.h"
#include "AnimationComponent.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "RenderComponent.h"
#include "BurgerPiece.h"
#include <limits>
#include <iostream>

namespace BurgerTime
{
    // ============================================
    // HELPER: Grid to Screen Conversion
    // ============================================
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

    // ============================================
    // LOAD LEVEL
    // ============================================
    void LevelManager::LoadLevel(int levelId, dae::Scene& scene)
    {
        std::cout << "\n=== Loading Level " << levelId << " ===\n";

        // Unload previous
        UnloadCurrentLevel(scene);

        // Load from JSON
        std::string filepath = "Data/BurgerTime/Levels/level" + std::to_string(levelId) + ".json";
        m_CurrentLevelData = LevelLoader::LoadFromFile(filepath, levelId);

        if (!m_CurrentLevelData)
        {
            std::cerr << "Failed to load level " << levelId << "\n";
            return;
        }

        m_CurrentLevelId = levelId;

        // Create level objects
        CreatePlatforms(scene);
        CreateLadders(scene);
		CreatePlates(scene);
        CreateBurgers(scene);
        CreateEnemies(scene);

        // Create player
        CreatePlayer(scene, 0); // Player 1

        std::cout << "Level " << levelId << " loaded\n\n";
    }

    void LevelManager::UnloadCurrentLevel(dae::Scene& scene)
    {
        if (m_CurrentLevelId == 0) return;
        scene.Clear();
        m_CurrentLevelData.reset();
        m_CurrentLevelId = 0;
        m_pPlayer1 = nullptr;
        m_BurgerPieces.clear();
        m_Enemies.clear();
    }

    dae::GameObject* LevelManager::CreatePlayer(dae::Scene& scene, int playerId)
    {
        if (!m_CurrentLevelData) return nullptr;

        glm::vec2 spawnPos = GridToScreen(m_CurrentLevelData->peterSpawn);

        if (!m_CurrentLevelData->platforms.empty())
        {
            float topY = std::numeric_limits<float>::max();
            for (const auto& plat : m_CurrentLevelData->platforms)
            {
                auto sp = GridToScreen(plat);
                if (sp.y >= 0.0f && sp.y < topY)
                    topY = sp.y;
            }

            if (topY < std::numeric_limits<float>::max())
            {
                float rightX = std::numeric_limits<float>::lowest();
                for (const auto& plat : m_CurrentLevelData->platforms)
                {
                    auto sp = GridToScreen(plat);
                    if (std::abs(sp.y - topY) < 5.0f && sp.x > rightX)
                        rightX = sp.x;
                }
                spawnPos = { rightX, topY - Config::PLAYER_HEIGHT };
            }
        }

        auto player = std::make_unique<dae::GameObject>();
        player->SetPosition(spawnPos.x, spawnPos.y);
        player->AddComponent<AnimationComponent>();
        player->AddComponent<Player>(playerId);
        player->AddComponent<dae::HealthComponent>(playerId, 1, 3);
        player->AddComponent<dae::ScoreComponent>(playerId);

        m_pPlayer1 = player->GetComponent<Player>();
        auto* playerPtr = player.get();
        scene.Add(std::move(player));
        return playerPtr;
    }

    dae::GameObject* LevelManager::GetPlayer1Object() const
    {
        return m_pPlayer1 ? m_pPlayer1->GetOwner() : nullptr;
    }

    void LevelManager::CreatePlatforms(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;

        std::cout << "\n=== Creating Platforms ===\n";

        for (size_t i = 0; i < std::min(m_CurrentLevelData->platforms.size(), size_t(5)); ++i)
        {
            const auto& pos = m_CurrentLevelData->platforms[i];
            auto screenPos = GridToScreen(pos);

            std::cout << "Platform " << i
                << ": Grid(" << pos.x << ", " << pos.y << ")"
                << " -> Screen(" << screenPos.x << ", " << screenPos.y << ")\n";

            auto platform = std::make_unique<dae::GameObject>();
            platform->SetPosition(screenPos.x, screenPos.y);

            auto* render = platform->AddComponent<dae::RenderComponent>("BurgerTime/World/platform.png");
            render->SetCustomSize(Config::PLATFORM_WIDTH, Config::PLATFORM_HEIGHT);

            scene.Add(std::move(platform));
        }

        // Create rest without debug output
        for (size_t i = 5; i < m_CurrentLevelData->platforms.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->platforms[i];
            auto screenPos = GridToScreen(pos);

            auto platform = std::make_unique<dae::GameObject>();
            platform->SetPosition(screenPos.x, screenPos.y);

            auto* render = platform->AddComponent<dae::RenderComponent>("BurgerTime/World/platform.png");
            render->SetCustomSize(Config::PLATFORM_WIDTH, Config::PLATFORM_HEIGHT);

            scene.Add(std::move(platform));
        }

        std::cout << "... Created " << m_CurrentLevelData->platforms.size() << " platforms\n";
    }

    void LevelManager::CreateLadders(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;

        for (const auto& pos : m_CurrentLevelData->ladders)
        {
            auto screenPos = GridToScreen(pos);

            auto ladder = std::make_unique<dae::GameObject>();
            ladder->SetPosition(screenPos.x, screenPos.y);

            auto* render = ladder->AddComponent<dae::RenderComponent>("BurgerTime/World/ladder.png");
            render->SetCustomSize(Config::LADDER_WIDTH, Config::LADDER_HEIGHT);

            scene.Add(std::move(ladder));
        }

        std::cout << "Created " << m_CurrentLevelData->ladders.size() << " ladders\n";
    }

    void LevelManager::CreatePlates(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;

        std::cout << "\n=== Creating Plates ===\n";

        for (size_t i = 0; i < m_CurrentLevelData->plates.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->plates[i];
            auto screenPos = GridToScreen(pos);

            std::cout << "Plate " << i
                << ": Grid(" << pos.x << ", " << pos.y << ")"
                << " -> Screen(" << screenPos.x << ", " << screenPos.y << ")\n";

            auto plate = std::make_unique<dae::GameObject>();
            plate->SetPosition(screenPos.x, screenPos.y);

            auto* render = plate->AddComponent<dae::RenderComponent>("BurgerTime/World/plate.png");
            render->SetCustomSize(Config::PLATE_WIDTH, Config::PLATE_HEIGHT);

            // Test: Try different pivot points
            // render->SetPivot(dae::PivotPoint::TopLeft);      // Default
            // render->SetPivot(dae::PivotPoint::BottomLeft);   // Test
            // render->SetPivot(dae::PivotPoint::Center);       // Test

            scene.Add(std::move(plate));
        }

        std::cout << "Created " << m_CurrentLevelData->plates.size() << " plates\n";
    }

    void LevelManager::CreateBurgers(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;

        std::cout << "\n=== Creating Burger Pieces ===\n";

        // TOP BUNS
        for (size_t i = 0; i < m_CurrentLevelData->topBuns.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->topBuns[i];
            auto screenPos = GridToScreen(pos);

            auto burgerPiece = std::make_unique<dae::GameObject>();
            burgerPiece->SetPosition(screenPos.x, screenPos.y);
            burgerPiece->AddComponent<BurgerPiece>(IngredientType::TOP_BUN);
            auto* piece = burgerPiece->GetComponent<BurgerPiece>();
            if (piece) m_BurgerPieces.push_back(piece);
            scene.Add(std::move(burgerPiece));
        }

        // LETTUCES
        for (size_t i = 0; i < m_CurrentLevelData->lettuces.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->lettuces[i];
            auto screenPos = GridToScreen(pos);

            auto burgerPiece = std::make_unique<dae::GameObject>();
            burgerPiece->SetPosition(screenPos.x, screenPos.y);
            burgerPiece->AddComponent<BurgerPiece>(IngredientType::LETTUCE);
            auto* piece = burgerPiece->GetComponent<BurgerPiece>();
            if (piece) m_BurgerPieces.push_back(piece);
            scene.Add(std::move(burgerPiece));
        }

        // PATTIES
        for (size_t i = 0; i < m_CurrentLevelData->patties.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->patties[i];
            auto screenPos = GridToScreen(pos);

            auto burgerPiece = std::make_unique<dae::GameObject>();
            burgerPiece->SetPosition(screenPos.x, screenPos.y);
            burgerPiece->AddComponent<BurgerPiece>(IngredientType::PATTY);
            auto* piece = burgerPiece->GetComponent<BurgerPiece>();
            if (piece) m_BurgerPieces.push_back(piece);
            scene.Add(std::move(burgerPiece));
        }

        // TOMATOES
        for (size_t i = 0; i < m_CurrentLevelData->tomatoes.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->tomatoes[i];
            auto screenPos = GridToScreen(pos);

            auto burgerPiece = std::make_unique<dae::GameObject>();
            burgerPiece->SetPosition(screenPos.x, screenPos.y);
            burgerPiece->AddComponent<BurgerPiece>(IngredientType::TOMATO);
            auto* piece = burgerPiece->GetComponent<BurgerPiece>();
            if (piece) m_BurgerPieces.push_back(piece);
            scene.Add(std::move(burgerPiece));
        }

        // CHEESES
        for (size_t i = 0; i < m_CurrentLevelData->cheeses.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->cheeses[i];
            auto screenPos = GridToScreen(pos);

            auto burgerPiece = std::make_unique<dae::GameObject>();
            burgerPiece->SetPosition(screenPos.x, screenPos.y);
            burgerPiece->AddComponent<BurgerPiece>(IngredientType::CHEESE);
            auto* piece = burgerPiece->GetComponent<BurgerPiece>();
            if (piece) m_BurgerPieces.push_back(piece);
            scene.Add(std::move(burgerPiece));
        }

        // BOTTOM BUNS
        for (size_t i = 0; i < m_CurrentLevelData->bottomBuns.size(); ++i)
        {
            const auto& pos = m_CurrentLevelData->bottomBuns[i];
            auto screenPos = GridToScreen(pos);

            auto burgerPiece = std::make_unique<dae::GameObject>();
            burgerPiece->SetPosition(screenPos.x, screenPos.y);
            burgerPiece->AddComponent<BurgerPiece>(IngredientType::BOTTOM_BUN);
            auto* piece = burgerPiece->GetComponent<BurgerPiece>();
            if (piece) m_BurgerPieces.push_back(piece);
            scene.Add(std::move(burgerPiece));
        }

        int total = m_CurrentLevelData->topBuns.size() +
            m_CurrentLevelData->lettuces.size() +
            m_CurrentLevelData->patties.size() +
            m_CurrentLevelData->tomatoes.size() +
            m_CurrentLevelData->cheeses.size() +
            m_CurrentLevelData->bottomBuns.size();

        std::cout << "Created " << total << " burger pieces\n";
    }


    void LevelManager::CreateEnemies(dae::Scene& scene)
    {
        if (!m_CurrentLevelData) return;

        for (const auto& pos : m_CurrentLevelData->enemySpawns)
        {
            auto screenPos = GridToScreen(pos);
            auto enemy = std::make_unique<dae::GameObject>();
            enemy->SetPosition(screenPos.x, screenPos.y);
            auto* enemyComp = enemy->AddComponent<Enemy>();
            if (m_pPlayer1)
                enemyComp->SetPlayerTarget(m_pPlayer1);
            if (enemyComp) m_Enemies.push_back(enemyComp);
            scene.Add(std::move(enemy));
        }
        std::cout << "Created " << m_CurrentLevelData->enemySpawns.size() << " enemies\n";
    }

    bool LevelManager::IsOnPlatform(float x, float y) const
    {
        if (!m_CurrentLevelData) return false;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;

        for (const auto& platform : m_CurrentLevelData->platforms)
        {
            auto sp = GridToScreen(platform);
            if (x >= sp.x - tol && x <= sp.x + tileSize + tol &&
                std::abs(feetY - sp.y) < tol)
                return true;
        }
        return false;
    }

    bool LevelManager::IsOnLadder(float x, float y) const
    {
        if (!m_CurrentLevelData) return false;
        const float xTol = Config::LADDER_X_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;

        for (const auto& ladder : m_CurrentLevelData->ladders)
        {
            auto sp = GridToScreen(ladder);
            if (feetY < sp.y || feetY > sp.y + tileSize) continue;
            if (x + Config::PLAYER_WIDTH <= sp.x - xTol) continue;  
            if (x >= sp.x + tileSize + xTol) continue; 

            return true;
        }
        return false;
    }

    bool LevelManager::IsPointOnPlatform(float x, float feetY) const
    {
        if (!m_CurrentLevelData) return false;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);

        for (const auto& platform : m_CurrentLevelData->platforms)
        {
            auto sp = GridToScreen(platform);
            if (x >= sp.x - tol && x <= sp.x + tileSize + tol &&
                std::abs(feetY - sp.y) < tol)
                return true;
        }
        return false;
    }

    bool LevelManager::IsLevelComplete() const
    {
        if (m_BurgerPieces.empty()) return false;
        for (auto* piece : m_BurgerPieces)
        {
            if (!piece->IsLanded()) return false;
        }
        return true;
    }
    float LevelManager::GetNearestPlatformY(float x, float y) const
    {
        if (!m_CurrentLevelData) return -1.0f;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;
        float bestDist = tol + 1.0f;
        float bestSurface = -1.0f;

        for (const auto& platform : m_CurrentLevelData->platforms)
        {
            auto sp = GridToScreen(platform);
            if (x < sp.x - tol || x > sp.x + tileSize + tol) continue;
            float dist = std::abs(feetY - sp.y);
            if (dist < tol && dist < bestDist)
            {
                bestDist = dist;
                bestSurface = sp.y;
            }
        }
        if (bestSurface < 0.0f) return -1.0f;
        return bestSurface - Config::PLAYER_HEIGHT;
    }

    float LevelManager::GetNearestLadderX(float x, float y) const
    {
        if (!m_CurrentLevelData) return -1.0f;
        const float xTol = Config::LADDER_X_TOLERANCE;
        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float feetY = y + Config::PLAYER_HEIGHT;
        float bestDist = 9999.0f;
        float bestX = -1.0f;

        for (const auto& ladder : m_CurrentLevelData->ladders)
        {
            auto sp = GridToScreen(ladder);
            if (feetY < sp.y || feetY > sp.y + tileSize) continue;
            if (x + Config::PLAYER_WIDTH <= sp.x - xTol) continue; 
            if (x >= sp.x + tileSize + xTol) continue;

            float snapX = sp.x + (tileSize - Config::PLAYER_WIDTH) * 0.5f;
            float dist = std::abs(x - snapX);
            if (dist < bestDist) { bestDist = dist; bestX = snapX; }
        }
        return bestX;
    }
}