#include "PlayerState.h"
#include "Player.h"
#include "AnimationComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include <iostream>

namespace BurgerTime
{
    // ============================================
    // IDLE STATE
    // ============================================
    void PlayerIdleState::OnEnter(Player* player)
    {
        std::cout << "Player " << player->GetPlayerId() << ": IDLE\n";
        player->PlayAnimation("Idle");
    }

    std::unique_ptr<PlayerState> PlayerIdleState::HandleInput(Player*, float dx, float dy)
    {
        if (dx != 0.0f || dy != 0.0f)
        {
            return std::make_unique<PlayerWalkingState>();
        }
        return nullptr;
    }

    // ============================================
    // WALKING STATE
    // ============================================
    void PlayerWalkingState::OnEnter(Player* player)
    {
        std::cout << "Player " << player->GetPlayerId() << ": WALKING\n";
        player->PlayAnimation("Walk");
    }

    void PlayerWalkingState::OnExit(Player*)
    {
    }

    std::unique_ptr<PlayerState> PlayerWalkingState::Update(Player*, float deltaTime)
    {
        m_WalkTimer += deltaTime;
        return nullptr;
    }

    std::unique_ptr<PlayerState> PlayerWalkingState::HandleInput(Player* player, float dx, float dy)
    {
        if (dy != 0.0f && player->IsOnLadder())
        {
            return std::make_unique<PlayerClimbingState>();
        }

        if (dx == 0.0f && dy == 0.0f)
        {
            return std::make_unique<PlayerIdleState>();
        }

        return nullptr;
    }

    // ============================================
    // CLIMBING STATE
    // ============================================
    void PlayerClimbingState::OnEnter(Player* player)
    {
        std::cout << "Player " << player->GetPlayerId() << ": CLIMBING\n";
        player->PlayAnimation("ClimbUp");
    }

    void PlayerClimbingState::OnExit(Player*)
    {
    }

    std::unique_ptr<PlayerState> PlayerClimbingState::Update(Player*, float deltaTime)
    {
        return nullptr;
    }

    std::unique_ptr<PlayerState> PlayerClimbingState::HandleInput(Player* player, float dx, float dy)
    {
        if (dy < 0)
        {
            player->PlayAnimation("ClimbUp");
        }
        else if (dy > 0)
        {
            player->PlayAnimation("ClimbDown");
        }

        if (!player->IsOnLadder())
        {
            if (dx != 0.0f)
                return std::make_unique<PlayerWalkingState>();
            else
                return std::make_unique<PlayerIdleState>();
        }

        if (dy == 0.0f && dx == 0.0f)
        {
            return std::make_unique<PlayerIdleState>();
        }

        return nullptr;
    }

    // ============================================
    // DEAD STATE
    // ============================================
    void PlayerDeadState::OnEnter(Player* player)
    {
        std::cout << "Player " << player->GetPlayerId() << ": DEAD!\n";
        player->PlayAnimation("Death");
        m_DeathTimer = 0.0f;
    }

    std::unique_ptr<PlayerState> PlayerDeadState::Update(Player* player, float deltaTime)
    {
        m_DeathTimer += deltaTime;

        auto* animComp = player->GetAnimationComponent();
        bool animFinished = animComp ? animComp->HasFinished() : true;

        if (animFinished || m_DeathTimer >= m_DeathDuration)
        {
            auto* health = player->GetOwner()->GetComponent<dae::HealthComponent>();

            if (health && health->GetLives() > 0)
            {
                player->Reset();
                return std::make_unique<PlayerIdleState>();
            }
            else
            {
                std::cout << "Player " << player->GetPlayerId() << " - GAME OVER\n";
                player->SetActive(false);
            }
        }

        return nullptr;
    }
}