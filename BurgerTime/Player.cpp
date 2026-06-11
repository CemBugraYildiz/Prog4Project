#include "Player.h"
#include "PlayerState.h"
#include "AnimationComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "ServiceLocator.h"
#include "SoundIds.h"
#include "EventQueue.h"
#include "Event.h"
#include "LevelManager.h"
#include <iostream>
#include "EngineTime.h"
#include "GameConfig.h"
#include "Enemy.h"


namespace BurgerTime
{
    // ============================================
    // CONSTRUCTOR
    // ============================================
    Player::Player(dae::GameObject* owner, int playerId)
        : Component(owner)
        , m_PlayerId(playerId)
        , m_CurrentState(std::make_unique<PlayerIdleState>())
    {
    }
    // ============================================
    // DESTRUCTOR
    // ============================================
    Player::~Player()
    {
        // PlayerState destructor burada çaðrýlýr
        // PlayerState.h include edildiði için artýk complete type
    }
    // ============================================
    // LIFECYCLE
    // ============================================
    void Player::OnAttach()
    {
        m_SpawnPosition = GetPosition();
        m_LastPosition = GetPosition();

        m_AnimationComp = GetOwner()->GetComponent<AnimationComponent>();
        if (!m_AnimationComp)
            m_AnimationComp = GetOwner()->AddComponent<AnimationComponent>();

        m_AnimationComp->SetRenderSize(Config::PLAYER_WIDTH, Config::PLAYER_HEIGHT);

        SetupAnimations();

        if (m_CurrentState)
            m_CurrentState->OnEnter(this);
    }

    void Player::OnDetach()
    {
        std::cout << "Player " << m_PlayerId << " detached\n";

        if (m_CurrentState)
        {
            m_CurrentState->OnExit(this);
        }
    }

    void Player::Update()
    {
        if (!IsActive()) return;

        // ============================================
        // HAREKET ALGILAMA
        // ============================================
        glm::vec2 currentPosition = GetPosition();
        glm::vec2 delta = currentPosition - m_LastPosition;

        float movementMagnitude = glm::length(delta);
        bool isMoving = movementMagnitude > 0.1f; // Threshold

        if (isMoving)
        {
            // Normalize direction for state input
            glm::vec2 direction = glm::normalize(delta);

            // State'e input bildir (dx, dy non-zero = moving)
            if (m_CurrentState)
            {
                auto nextState = m_CurrentState->HandleInput(this, direction.x * 100.f, direction.y * 100.f);

                if (nextState)
                {
                    TransitionTo(std::move(nextState));
                }
            }

            // Update facing direction
            if (std::abs(delta.x) > std::abs(delta.y))
            {
                // Horizontal movement dominant
                if (delta.x < 0)
                    SetFacingDirection(Direction::Left);
                else
                    SetFacingDirection(Direction::Right);
            }
            else
            {
                // Vertical movement dominant
                if (delta.y < 0)
                    SetFacingDirection(Direction::Up);
                else
                    SetFacingDirection(Direction::Down);
            }
        }
        else
        {
            // No movement - transition to Idle
            if (m_CurrentState)
            {
                auto nextState = m_CurrentState->HandleInput(this, 0.f, 0.f);

                if (nextState)
                {
                    TransitionTo(std::move(nextState));
                }
            }
        }

        // Store current position for next frame
        m_LastPosition = currentPosition;

        if (m_IsInvincible)
        {
            m_InvincibleTimer -= dae::EngineTime::GetDeltaTime();
            if (m_InvincibleTimer <= 0.0f)
                m_IsInvincible = false;
        }

        // ============================================
        // STATE UPDATE (time-based state changes)
        // ============================================
        if (m_CurrentState)
        {
            auto nextState = m_CurrentState->Update(this, dae::EngineTime::GetDeltaTime());

            if (nextState)
            {
                TransitionTo(std::move(nextState));
            }
        }
    }

    void Player::Render() const
    {
        // Rendering handled by AnimationComponent
    }

    // ============================================
    // SETUP ANIMATIONS
    // ============================================
    void Player::SetupAnimations()
    {
        if (!m_AnimationComp) return;

        AnimationClip idle;
        idle.name = "Idle";
        idle.texturePath = "BurgerTime/PeterPepper/Peter_Idle.png";
        idle.frameCount = 1;
        idle.rows = 1;
        idle.columns = 1;
        idle.frameTime = 0.1f;
        idle.loop = true;
        idle.flipX = false;
        m_AnimationComp->AddAnimation(idle);

        AnimationClip walk;
        walk.name = "Walk";
        walk.texturePath = "BurgerTime/PeterPepper/Peter_Walk.png";
        walk.frameCount = 3;
        walk.rows = 1;
        walk.columns = 3;
        walk.frameTime = 0.1f;
        walk.loop = true;
        walk.flipX = false;
        m_AnimationComp->AddAnimation(walk);

        AnimationClip climbUp;
        climbUp.name = "ClimbUp";
        climbUp.texturePath = "BurgerTime/PeterPepper/Peter_ClimbUp.png";
        climbUp.frameCount = 3;
        climbUp.rows = 1;
        climbUp.columns = 3;
        climbUp.frameTime = 0.1f;
        climbUp.loop = true;
        climbUp.flipX = false;
        m_AnimationComp->AddAnimation(climbUp);

        AnimationClip climbDown;
        climbDown.name = "ClimbDown";
        climbDown.texturePath = "BurgerTime/PeterPepper/Peter_ClimbDown.png";
        climbDown.frameCount = 3;
        climbDown.rows = 1;
        climbDown.columns = 3;
        climbDown.frameTime = 0.1f;
        climbDown.loop = true;
        climbDown.flipX = false;
        m_AnimationComp->AddAnimation(climbDown);

        AnimationClip death;
        death.name = "Death";
        death.texturePath = "BurgerTime/PeterPepper/Peter_Death.png";
        death.frameCount = 5;
        death.rows = 1;
        death.columns = 5;
        death.frameTime = 0.15f;
        death.loop = false;
        death.flipX = false;
        m_AnimationComp->AddAnimation(death);

        AnimationClip victory;
        victory.name = "Victory";
        victory.texturePath = "BurgerTime/PeterPepper/Peter_Victory.png";
        victory.frameCount = 2;
        victory.rows = 1;
        victory.columns = 2;
        victory.frameTime = 0.2f;
        victory.loop = true;
        victory.flipX = false;
        m_AnimationComp->AddAnimation(victory);

        m_AnimationComp->Play("Idle");
    }

    // ============================================
    // PUBLIC API
    // ============================================
    void Player::Move(float dx, float dy, float deltaTime)
    {
        if (!CanMove()) return;

        auto& lm = LevelManager::GetInstance();
        auto currentPos = GetPosition();
        glm::vec2 newPos = currentPos;

        if (dx != 0.0f && dy == 0.0f)
        {
            if (!lm.IsOnPlatform(currentPos.x, currentPos.y)) return;

            newPos.x = currentPos.x + dx * deltaTime;

            float snapY = lm.GetNearestPlatformY(currentPos.x, currentPos.y);
            if (snapY >= 0.0f) newPos.y = snapY;

            if (!lm.IsOnPlatform(newPos.x, newPos.y)) return;

            if (dx > 0.0f)
            {
                const float feetY = newPos.y + Config::PLAYER_HEIGHT;
                if (!lm.IsPointOnPlatform(newPos.x + Config::PLAYER_WIDTH, feetY)) return;
            }
        }
        else if (dy != 0.0f && dx == 0.0f)
        {
            if (!lm.IsOnLadder(currentPos.x, currentPos.y)) return;

            newPos.y = currentPos.y + dy * deltaTime;

            float snapX = lm.GetNearestLadderX(currentPos.x, currentPos.y);
            if (snapX >= 0.0f) newPos.x = snapX;
        }
        else return;

        if (CanMoveTo(newPos))
        {
            GetOwner()->SetPosition(newPos.x, newPos.y);

            if (dx < 0)      SetFacingDirection(Direction::Left);
            else if (dx > 0) SetFacingDirection(Direction::Right);
            else if (dy < 0) SetFacingDirection(Direction::Up);
            else if (dy > 0) SetFacingDirection(Direction::Down);
        }
    }

    void Player::UsePepper()
    {
        if (!CanUsePepper() || m_PepperCount <= 0)
        {
            std::cout << "Player " << m_PlayerId << ": No pepper!\n";
            return;
        }
        m_PepperCount--;

        dae::ServiceLocator::GetSoundSystem().Play(dae::SOUND_HIT, 1.0f);

        auto pos = GetPosition();
        const float range = 64.0f;
        float pepperLeft = (m_FacingDirection == Direction::Left)
            ? pos.x - range : pos.x + Config::PLAYER_WIDTH;
        float pepperRight = pepperLeft + range;
        float pepperTop = pos.y - 8.0f;
        float pepperBot = pos.y + Config::PLAYER_HEIGHT + 8.0f;

        int stunCount = 0;
        for (auto* enemy : LevelManager::GetInstance().GetEnemies())
        {
            if (!enemy) continue;
            auto ePos = enemy->GetOwner()->GetWorldPosition();
            if (ePos.x + Config::ENEMY_WIDTH > pepperLeft &&
                ePos.x                        < pepperRight &&
                ePos.y + Config::ENEMY_HEIGHT > pepperTop &&
                ePos.y < pepperBot
                )
            {
                enemy->OnPepperHit();
                ++stunCount;
            }
        }

        dae::Event evt;
        evt.type = dae::EventType::PepperUsed;
        evt.playerId = m_PlayerId;
        evt.value = m_PepperCount;
        dae::EventQueue::GetInstance().QueueEvent(evt);

        std::cout << "Pepper used! Stunned " << stunCount
            << " enemies. Remaining: " << m_PepperCount << "\n";
    }

    void Player::TakeDamage()
    {
        if (m_IsInvincible) return;

        m_IsInvincible = true;
        m_InvincibleTimer = INVINCIBLE_DURATION;

        std::cout << "Player " << m_PlayerId << " took damage!\n";

        auto* health = GetOwner()->GetComponent<dae::HealthComponent>();
        if (health)
        {
            health->TakeDamage(1); 
            if (health->GetLives() <= 0)
                TransitionTo(std::make_unique<PlayerDeadState>());
        }

        dae::ServiceLocator::GetSoundSystem().Play(dae::SOUND_HIT, 1.0f);
    }

    void Player::AddScore(int points)
    {
        auto* score = GetOwner()->GetComponent<dae::ScoreComponent>();
        if (score)
        {
            score->AddScore(points);
        }
    }

    void Player::Reset()
    {
        std::cout << "Player " << m_PlayerId << " respawning...\n";

        GetOwner()->SetPosition(m_SpawnPosition.x, m_SpawnPosition.y);
        m_PepperCount = 5;

        TransitionTo(std::make_unique<PlayerIdleState>());
    }

    // ============================================
    // GETTERS
    // ============================================
    glm::vec2 Player::GetPosition() const
    {
        auto pos = GetOwner()->GetWorldPosition();
        return glm::vec2(pos.x, pos.y);
    }

    bool Player::IsAlive() const
    {
        auto* health = GetOwner()->GetComponent<dae::HealthComponent>();
        return health ? health->GetLives() > 0 : true;
    }

    bool Player::CanMove() const
    {
        return m_CurrentState ? m_CurrentState->CanMove() : false;
    }

    bool Player::CanUsePepper() const
    {
        return m_CurrentState ? m_CurrentState->CanUsePepper() : false;
    }

    // ============================================
    // COLLISION QUERIES
    // ============================================
    bool Player::IsOnPlatform() const
    {
        auto pos = GetPosition();
        return BurgerTime::LevelManager::GetInstance().IsOnPlatform(pos.x, pos.y);
    }

    bool Player::IsOnLadder() const
    {
        auto pos = GetPosition();
        return BurgerTime::LevelManager::GetInstance().IsOnLadder(pos.x, pos.y);
    }

    bool Player::CanMoveTo(const glm::vec2& position) const
    {
        const float minX = 0.0f;
        const float minY = 0.0f;
        const float maxX = static_cast<float>(Config::WINDOW_WIDTH) - Config::PLAYER_WIDTH;
        const float maxY = static_cast<float>(Config::WINDOW_HEIGHT) - Config::PLAYER_HEIGHT;

        return position.x >= minX && position.x <= maxX &&
            position.y >= minY && position.y <= maxY;
    }

    // ============================================
    // STATE MANAGEMENT
    // ============================================
    void Player::TransitionTo(std::unique_ptr<PlayerState> newState)
    {
        if (!newState) return;

        if (m_CurrentState)
        {
            m_CurrentState->OnExit(this);
        }

        m_CurrentState = std::move(newState);

        if (m_CurrentState)
        {
            m_CurrentState->OnEnter(this);
        }
    }

    // ============================================
    // ANIMATION
    // ============================================
    void Player::PlayAnimation(const std::string& animName)
    {
        if (m_AnimationComp)
        {
            m_AnimationComp->Play(animName);
        }
    }

    void Player::SetFacingDirection(Direction dir)
    {
        m_FacingDirection = dir;

        if (m_AnimationComp)
        {
            if (dir == Direction::Left)
            {
                m_AnimationComp->SetFlipX(false);
            }
            else if (dir == Direction::Right)
            {
                m_AnimationComp->SetFlipX(true);
            }
        }
    }

}