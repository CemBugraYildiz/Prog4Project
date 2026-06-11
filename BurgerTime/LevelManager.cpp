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
#include <queue>
#include <algorithm>

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
        

        CreatePlayer(scene, 0); // Player 1
        BuildNavGraph();
        CreateEnemies(scene);

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
        spawnPos.y -= Config::PLAYER_HEIGHT;

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
            screenPos.y -= Config::ENEMY_HEIGHT;

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

    void LevelManager::RemoveEnemy(Enemy* enemy)
    {
        auto it = std::find(m_Enemies.begin(), m_Enemies.end(), enemy);
        if (it != m_Enemies.end())
            m_Enemies.erase(it);
    }

    void LevelManager::BuildNavGraph()
    {
        m_Sections.clear();
        m_NavGraph.clear();
        if (!m_CurrentLevelData) return;

        const float tileSize = static_cast<float>(Config::TILE_SIZE);
        const float tol = Config::PLATFORM_SNAP_TOLERANCE;
        const float enemyW = Config::ENEMY_WIDTH;

        std::map<float, std::vector<float>> tilesByY;
        for (const auto& plat : m_CurrentLevelData->platforms)
        {
            auto sp = GridToScreen(plat);
            tilesByY[sp.y].push_back(sp.x);
        }
        for (auto& [y, xs] : tilesByY)
            std::sort(xs.begin(), xs.end());

        int nextId = 0;
        for (const auto& [surfY, xs] : tilesByY)
        {
            int i = 0;
            while (i < (int)xs.size())
            {
                float xMin = xs[i];
                float xMax = xs[i] + tileSize;
                while (i + 1 < (int)xs.size() && xs[i + 1] <= xMax + 1.0f)
                {
                    ++i;
                    xMax = xs[i] + tileSize;
                }
                m_Sections.push_back({ nextId++, surfY, xMin - tol, xMax + tol });
                ++i;
            }
        }

        std::map<int, std::vector<int>> byX;
        for (const auto& lad : m_CurrentLevelData->ladders)
            byX[lad.x].push_back(lad.y);
        for (auto& [x, ys] : byX)
            std::sort(ys.begin(), ys.end());

        for (const auto& [gridX, gridYs] : byX)
        {
            float screenX = static_cast<float>(gridX) + Config::LEVEL_OFFSET_X;
            float snapX = screenX + (tileSize - enemyW) * 0.5f;

            int i = 0;
            while (i < (int)gridYs.size())
            {
                int segEnd = i;
                while (segEnd + 1 < (int)gridYs.size() &&
                    gridYs[segEnd + 1] == gridYs[segEnd] + (int)tileSize)
                    ++segEnd;

                float segTopY = static_cast<float>(gridYs[i]);
                float segBotY = static_cast<float>(gridYs[segEnd]) + tileSize;

                std::vector<int> sects;
                for (const auto& sec : m_Sections)
                {
                    if (sec.surfaceY < segTopY - tol || sec.surfaceY > segBotY + tol) continue;
                    if (snapX < sec.xMin || snapX > sec.xMax) continue;
                    sects.push_back(sec.id);
                }

                for (int a = 0; a < (int)sects.size(); ++a)
                    for (int b = a + 1; b < (int)sects.size(); ++b)
                    {
                        m_NavGraph[sects[a]].push_back({ snapX, sects[b] });
                        m_NavGraph[sects[b]].push_back({ snapX, sects[a] });
                    }

                i = segEnd + 1;
            }
        }
    }

    int LevelManager::GetEntitySection(float x, float y) const
    {
        const float feetY = y + Config::PLAYER_HEIGHT;
        const float tol = Config::PLATFORM_SNAP_TOLERANCE * 2.0f;
        for (const auto& sec : m_Sections)
        {
            if (std::abs(feetY - sec.surfaceY) > tol) continue;
            if (x >= sec.xMin && x <= sec.xMax) return sec.id;
        }
        return -1;
    }

    const LevelManager::PlatSection* LevelManager::GetSection(int id) const
    {
        if (id < 0 || id >= (int)m_Sections.size()) return nullptr;
        return &m_Sections[id];
    }

    std::vector<LevelManager::NavEdge>
        LevelManager::FindEnemyPath(int fromSection, int toSection) const
    {
        if (fromSection < 0 || toSection < 0 || fromSection == toSection) return {};

        std::map<int, int>      parent;
        std::map<int, NavEdge>  edgeUsed;
        std::queue<int>         q;

        parent[fromSection] = fromSection;
        q.push(fromSection);

        while (!q.empty())
        {
            int cur = q.front(); q.pop();

            if (cur == toSection)
            {
                std::vector<NavEdge> path;
                int node = cur;
                while (node != fromSection)
                {
                    path.push_back(edgeUsed[node]);
                    node = parent[node];
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            auto it = m_NavGraph.find(cur);
            if (it == m_NavGraph.end()) continue;

            for (const auto& edge : it->second)
            {
                if (parent.find(edge.destSectionId) == parent.end())
                {
                    parent[edge.destSectionId] = cur;
                    edgeUsed[edge.destSectionId] = edge;
                    q.push(edge.destSectionId);
                }
            }
        }
        return {};
    }
}