#include "BurgerPiece.h"
#include "GameConfig.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "LevelManager.h"
#include "Player.h"
#include "EngineTime.h"
#include "EventQueue.h"
#include "Enemy.h"
#include <iostream>

namespace BurgerTime
{
    BurgerPiece::BurgerPiece(dae::GameObject* owner, IngredientType type)
        : Component(owner)
        , m_Type(type)
    {
    }

    void BurgerPiece::OnAttach()
    {
        std::cout << "BurgerPiece attached: " << GetSpriteBaseName()
            << " at (" << GetOwner()->GetWorldPosition().x
            << ", " << GetOwner()->GetWorldPosition().y << ")\n";

        //Segments are created here, AFTER parent position is set
        CreateSegments();
    }

    void BurgerPiece::CreateSegments()
    {
        std::string baseName = GetSpriteBaseName();

        // Parent GameObject position is already in SCREEN SPACE (GridToScreen applied)
        // We just need RELATIVE offset for segments

        const float segmentWidth = Config::BURGER_SEGMENT_WIDTH;

        for (int i = 0; i < 4; ++i)
        {
            auto segment = std::make_unique<dae::GameObject>();
            segment->SetPosition(
                i * segmentWidth,  
                0.0f               
            );

            std::string spritePath = "BurgerTime/Ingredients/" + baseName + std::to_string(i) + ".png";
            auto* render = segment->AddComponent<dae::RenderComponent>(spritePath);

            render->SetCustomSize(Config::BURGER_SEGMENT_WIDTH, Config::BURGER_PIECE_HEIGHT);

            m_SegmentObjects[i] = segment.get();
            GetOwner()->AttachChild(std::move(segment));

            auto segmentWorldPos = m_SegmentObjects[i]->GetWorldPosition();
            std::cout << "    Segment " << i << " world pos: ("
                << segmentWorldPos.x << ", " << segmentWorldPos.y << ")\n";
        }

        std::cout << "Created 4 segments for " << baseName
            << " at parent pos (" << GetOwner()->GetWorldPosition().x
            << ", " << GetOwner()->GetWorldPosition().y << ")\n";
    }

    void BurgerPiece::Update()
    {
        if (!IsActive()) return;

        if (!m_IsFalling && !m_IsLanded)
        {
            auto* player1 = LevelManager::GetInstance().GetPlayer1();
            if (player1) CheckPlayerWalkOver(player1);
        }

        if (m_IsFalling && !m_IsLanded)
        {
            auto currentPos = GetOwner()->GetWorldPosition();

            float bLeft = currentPos.x;
            float bRight = currentPos.x + 4.0f * Config::BURGER_SEGMENT_WIDTH;
            float bTop = currentPos.y;
            float bBottom = currentPos.y + Config::BURGER_PIECE_HEIGHT;

            for (auto* enemy : LevelManager::GetInstance().GetEnemies())
            {
                if (!enemy) continue;
                auto ePos = enemy->GetOwner()->GetWorldPosition();
                if (bRight > ePos.x &&
                    bLeft   < ePos.x + Config::ENEMY_WIDTH &&
                    bBottom > ePos.y &&
                    bTop < ePos.y)
                {
                    enemy->OnBurgerCrush();
                    ++m_EnemiesCrushedThisFall;
                }
            }

            float newY = currentPos.y + m_FallSpeed * dae::EngineTime::GetDeltaTime();

            if (newY >= m_TargetY)
            {
                newY = m_TargetY;
                m_IsFalling = false;

                auto* player = LevelManager::GetInstance().GetPlayer1();
                if (m_EnemiesCrushedThisFall > 0 && player)
                {
                    int killScore = (m_EnemiesCrushedThisFall == 1) ? 500 :
                        (m_EnemiesCrushedThisFall == 2) ? 1000 : 2000;
                    player->AddScore(killScore);
                }
                m_EnemiesCrushedThisFall = 0;

                if (m_TargetIsPlate)
                {
                    float finalY = CalculatePlateStackY();
                    GetOwner()->SetPosition(currentPos.x, finalY);
                    m_IsLanded = true;

                    auto* player = LevelManager::GetInstance().GetPlayer1();
                    if (player) player->AddScore(50);

                    if (LevelManager::GetInstance().IsLevelComplete())
                    {
                        dae::Event evt;
                        evt.type = dae::EventType::LevelCompleted;
                        dae::EventQueue::GetInstance().QueueEvent(evt);
                        std::cout << "=== LEVEL COMPLETE! ===\n";
                    }
                }
                else
                {
                    GetOwner()->SetPosition(currentPos.x, newY);
                    m_SegmentWalked.fill(false);
                    m_WalkedCount = 0;
                    UpdateSegmentVisuals();
                    TriggerChainReaction(newY);
                }
            }
            else
            {
                GetOwner()->SetPosition(currentPos.x, newY);
            }
        }
    }

    void BurgerPiece::Render() const
    {
        // Segments render themselves via RenderComponent
    }

    // ============================================
    // PLAYER INTERACTION
    // ============================================
    void BurgerPiece::OnPlayerWalkOver(int segmentIndex)
    {
        if (segmentIndex < 0 || segmentIndex >= 4) return;
        if (m_SegmentWalked[segmentIndex]) return; // Already walked
        if (m_IsFalling || m_IsLanded) return; // Can't walk on falling piece

        std::cout << "Player walked on segment " << segmentIndex << "\n";

        m_SegmentWalked[segmentIndex] = true;
        m_WalkedCount++;

        // Visual feedback - could shake or highlight segment
        UpdateSegmentVisuals();

        // Check if all segments walked
        if (IsFullyWalked())
        {
            std::cout << "All segments walked! Piece falling...\n";
            StartFalling();
        }
    }

    bool BurgerPiece::IsFullyWalked() const
    {
        return m_WalkedCount >= 4;
    }

    void BurgerPiece::StartFalling()
    {
        if (m_IsFalling) return;
        m_IsFalling = true;
        m_EnemiesCrushedThisFall = 0;

        bool isPlate = false;
        float surfaceY = GetNextPlatformY(isPlate);
        m_TargetIsPlate = isPlate;
        m_TargetY = surfaceY - Config::INGREDIENT_PLATFORM_OFFSET;

        const float maxY = Config::BURGER_MAX_FALL_Y;
        if (m_TargetY > maxY) { m_TargetY = maxY; m_TargetIsPlate = true; }

        std::cout << "Burger falling to Y=" << m_TargetY
            << (m_TargetIsPlate ? " [PLATE]" : " [platform]") << "\n";
    }

    // ============================================
    // HELPER METHODS
    // ============================================
    std::string BurgerPiece::GetSpriteBaseName() const
    {
        switch (m_Type)
        {
        case IngredientType::TOP_BUN:      return "bun";
        case IngredientType::BOTTOM_BUN:   return "bottombun";
        case IngredientType::LETTUCE:      return "lettuce";
        case IngredientType::TOMATO:       return "tomato";
        case IngredientType::CHEESE:       return "cheese";
        case IngredientType::PATTY:        return "patty";
        default:                           return "bun";
        }
    }

    float BurgerPiece::GetNextPlatformY(bool& outIsPlate) const
    {
        auto currentPos = GetOwner()->GetWorldPosition();
        const auto* levelData = LevelManager::GetInstance().GetCurrentLevelData();
        if (!levelData) { outIsPlate = false; return currentPos.y + 300.0f; }

        const float TILE = static_cast<float>(Config::TILE_SIZE);
        const float PLATE_W = Config::PLATE_WIDTH; 
        const float MIN_OVERLAP = 8.0f;              
        const float burgerLeft = currentPos.x;
        const float burgerRight = currentPos.x + 4.0f * Config::BURGER_SEGMENT_WIDTH;

        float closestY = currentPos.y + 2000.0f;
        bool  closestPlate = false;

        for (const auto& platform : levelData->platforms)
        {
            auto sp = LevelManager::GridToScreen(platform);
            if (sp.y <= currentPos.y + Config::INGREDIENT_PLATFORM_OFFSET + 4.0f) continue;

            float overlapLeft = std::max(sp.x, burgerLeft);
            float overlapRight = std::min(sp.x + TILE, burgerRight);
            if (overlapRight - overlapLeft < MIN_OVERLAP) continue;

            if (sp.y < closestY) { closestY = sp.y; closestPlate = false; }
        }

        for (const auto& plate : levelData->plates)
        {
            auto sp = LevelManager::GridToScreen(plate);
            if (sp.y <= currentPos.y + Config::INGREDIENT_PLATFORM_OFFSET + 4.0f) continue;

            float overlapLeft = std::max(sp.x, burgerLeft);
            float overlapRight = std::min(sp.x + PLATE_W, burgerRight);
            if (overlapRight - overlapLeft < MIN_OVERLAP) continue;

            if (sp.y < closestY) { closestY = sp.y; closestPlate = true; }
        }

        if (closestY > currentPos.y + 1000.0f)
        {
            closestY = Config::BURGER_MAX_FALL_Y;
            closestPlate = true;
        }

        outIsPlate = closestPlate;
        return closestY;
    }

    void BurgerPiece::UpdateSegmentVisuals()
    {
        for (int i = 0; i < 4; ++i)
        {
            if (m_SegmentObjects[i])
            {
                float yOffset = m_SegmentWalked[i] ? 4.0f : 0.0f;
                m_SegmentObjects[i]->SetPosition(
                    static_cast<float>(i) * Config::BURGER_SEGMENT_WIDTH,
                    yOffset
                );
            }
        }
    }
    void BurgerPiece::CheckPlayerWalkOver(Player* player)
    {
        auto piecePos = GetOwner()->GetWorldPosition();
        auto playerPos = player->GetPosition();

        float playerFeetY = playerPos.y + Config::PLAYER_HEIGHT;
        float diff = piecePos.y - playerFeetY;
        if (std::abs(diff) > 32.0f) return;

        float pieceLeft = piecePos.x;
        float pieceRight = piecePos.x + 4.0f * Config::BURGER_SEGMENT_WIDTH;
        if (playerPos.x < pieceLeft || playerPos.x > pieceRight) return;

        float relX = playerPos.x - pieceLeft;
        int segIdx = static_cast<int>(relX / Config::BURGER_SEGMENT_WIDTH);
        segIdx = std::max(0, std::min(3, segIdx));

        OnPlayerWalkOver(segIdx);
    }

    void BurgerPiece::TriggerChainReaction(float landedY)
    {
        auto& lm = LevelManager::GetInstance();
        auto myPos = GetOwner()->GetWorldPosition();

        const float burgerW = 4.0f * Config::BURGER_SEGMENT_WIDTH;
        const float myLeft = myPos.x;
        const float myRight = myPos.x + burgerW;

        for (auto* other : lm.GetBurgerPieces())
        {
            if (other == this)          continue;
            if (other->IsFalling())     continue;
            if (other->IsLanded())      continue;

            auto otherPos = other->GetOwner()->GetWorldPosition();

            if (std::abs(otherPos.y - landedY) > 4.0f) continue;

            float otherLeft = otherPos.x;
            float otherRight = otherPos.x + burgerW;
            if (myLeft >= otherRight || myRight <= otherLeft) continue;

            std::cout << "Chain reaction! Triggering burger at ("
                << otherPos.x << ", " << otherPos.y << ")\n";
            other->StartFalling();
        }
    }

    float BurgerPiece::CalculatePlateStackY() const
    {
        const float burgerW = 4.0f * Config::BURGER_SEGMENT_WIDTH;
        auto myPos = GetOwner()->GetWorldPosition();
        const float myLeft = myPos.x;
        const float myRight = myPos.x + burgerW;

        float topMostY = m_TargetY + 1.0f;

        for (auto* other : LevelManager::GetInstance().GetBurgerPieces())
        {
            if (other == this)      continue;
            if (!other->IsLanded()) continue;

            auto otherPos = other->GetOwner()->GetWorldPosition();

            float otherRight = otherPos.x + burgerW;
            if (myLeft >= otherRight || myRight <= otherPos.x) continue;

            if (std::abs(otherPos.y - m_TargetY) > static_cast<float>(Config::TILE_SIZE) * 4) continue;

            if (otherPos.y < topMostY)
                topMostY = otherPos.y;
        }

        if (topMostY <= m_TargetY)
            return topMostY - Config::BURGER_PIECE_HEIGHT;

        return m_TargetY;
    }
}