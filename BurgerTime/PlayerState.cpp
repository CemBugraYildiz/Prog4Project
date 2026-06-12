#include "PlayerState.h"
#include "Player.h"
#include "AnimationComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "Event.h"
#include "EventQueue.h"
#include "ScoreComponent.h"
#include "LevelManager.h"
#include "GameManager.h"
#include "GameMode.h"

namespace BurgerTime
{
    // IDLE STATE
    void PlayerIdleState::OnEnter(Player* player)
    {
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

    // WALKING STATE
    void PlayerWalkingState::OnEnter(Player* player)
    {
        player->PlayAnimation("Walk");
    }

    void PlayerWalkingState::OnExit(Player*)
    {
    }

    std::unique_ptr<PlayerState> PlayerWalkingState::Update(Player*, float deltaTime)
    {
        (void)deltaTime;
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

    // CLIMBING STATE
    void PlayerClimbingState::OnEnter(Player* player)
    {
        player->PlayAnimation("ClimbUp");
    }

    void PlayerClimbingState::OnExit(Player*)
    {
    }

    std::unique_ptr<PlayerState> PlayerClimbingState::Update(Player*, float /*deltaTime*/)
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

    // DEAD STATE
    void PlayerDeadState::OnEnter(Player* player)
    {
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
                player->SetActive(false);

                bool shouldFireGameOver = true;
                int  gameOverScore = 0;

                if (GameManager::GetInstance().GetGameMode() == GameMode::CoOp)
                {
                    auto* p1 = LevelManager::GetInstance().GetPlayer1();
                    auto* p2 = LevelManager::GetInstance().GetPlayer2();

                    bool anyoneAlive = (p1 && p1->IsAlive()) || (p2 && p2->IsAlive());
                    if (anyoneAlive)
                        shouldFireGameOver = false;

                    if (p1)
                    {
                        auto* sc = p1->GetOwner()->GetComponent<dae::ScoreComponent>();
                        gameOverScore = sc ? sc->GetScore() : 0;
                    }
                }
                else
                {
                    auto* sc = player->GetOwner()->GetComponent<dae::ScoreComponent>();
                    gameOverScore = sc ? sc->GetScore() : 0;
                }

                if (shouldFireGameOver)
                {
                    dae::Event evt;
                    evt.type = dae::EventType::GameOver;
                    evt.value = gameOverScore;
                    dae::EventQueue::GetInstance().QueueEvent(evt);
                }
            }
        }
        return nullptr;
    }

    void PlayerVictoryState::OnEnter(Player* player)
    {
        player->PlayAnimation("Victory");
    }

    std::unique_ptr<PlayerState> PlayerVictoryState::Update(Player*, float dt)
    {
        m_Timer += dt;
        if (!m_Fired && m_Timer >= DURATION)
        {
            m_Fired = true;
            dae::Event evt;
            evt.type = dae::EventType::RequestNextLevel;
            dae::EventQueue::GetInstance().QueueEvent(evt);
        }
        return nullptr;
    }
}