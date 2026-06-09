#include "Enemy.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "SceneManager.h"
#include "ScoreComponent.h"
#include "Player.h"
#include "EngineTime.h"
#include "GameConfig.h"
#include <iostream>

namespace BurgerTime
{
    void NormalState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: Normal state\n";
        enemy->SetSprite("enemy_normal.png");
    }

    std::unique_ptr<EnemyState> NormalState::Update(Enemy* enemy)
    {
        auto playerPos = enemy->GetPlayerPosition();
        auto enemyPos = enemy->GetPosition();

        glm::vec2 direction = playerPos - enemyPos;
        float distance = glm::length(direction);

        if (distance > 1.0f)
        {
            direction = glm::normalize(direction);
            float dt = dae::EngineTime::GetDeltaTime();
            enemy->Move(direction.x * m_Speed * dt, direction.y * m_Speed * dt);
        }
        return nullptr;
    }

    std::unique_ptr<EnemyState> NormalState::OnPepperHit(Enemy*)
    {
        return std::make_unique<StunnedState>();
    }

    std::unique_ptr<EnemyState> NormalState::OnBurgerCrush(Enemy*)
    {
        return std::make_unique<CrushedState>();
    }

    void StunnedState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: STUNNED!\n";
        m_StunTimer = 0.0f;
        enemy->SetSprite("enemy_stunned.png");
    }

    void StunnedState::OnExit(Enemy*)
    {
        std::cout << "Enemy: Recovered from stun\n";
    }

    std::unique_ptr<EnemyState> StunnedState::Update(Enemy*)
    {
        m_StunTimer += dae::EngineTime::GetDeltaTime();
        if (m_StunTimer >= m_StunDuration)
            return std::make_unique<NormalState>();
        return nullptr;
    }

    std::unique_ptr<EnemyState> StunnedState::OnPepperHit(Enemy*)
    {
        m_StunTimer = 0.0f;
        std::cout << "Stun duration reset!\n";
        return nullptr;
    }

    std::unique_ptr<EnemyState> StunnedState::OnBurgerCrush(Enemy*)
    {
        return std::make_unique<CrushedState>();
    }

    void CrushedState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: CRUSHED by burger!\n";
        enemy->SetSprite("enemy_crushed.png");
        enemy->AwardPoints(500);
    }

    std::unique_ptr<EnemyState> CrushedState::Update(Enemy* enemy)
    {
        enemy->Move(0.0f, m_FallSpeed * dae::EngineTime::GetDeltaTime());
        if (enemy->IsOnGround())
            return std::make_unique<EnemyDeadState>();
        return nullptr;
    }

    void EnemyDeadState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: DEAD!\n";
        enemy->SetSprite("enemy_dead.png");
    }

    std::unique_ptr<EnemyState> EnemyDeadState::Update(Enemy* enemy)
    {
        m_DeathTimer += dae::EngineTime::GetDeltaTime();
        if (m_DeathTimer >= m_DeathDuration)
            enemy->DestroyEnemy();
        return nullptr;
    }

    Enemy::Enemy(dae::GameObject* owner)
        : Component(owner)
        , m_CurrentState(std::make_unique<NormalState>())
    {
    }

    void Enemy::OnAttach()
    {
        if (m_CurrentState)
        {
            m_CurrentState->OnEnter(this);
        }
    }

    void Enemy::OnDetach()
    {
        if (m_CurrentState)
        {
            m_CurrentState->OnExit(this);
        }
    }

    void Enemy::Update()
    {
        if (!IsActive()) return;

        if (m_CurrentState)
        {
            auto nextState = m_CurrentState->Update(this);

            if (nextState)
            {
                TransitionTo(std::move(nextState));
            }
        }
    }

    void Enemy::Render() const
    {

    }

    void Enemy::TransitionTo(std::unique_ptr<EnemyState> newState)
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

    void Enemy::OnPepperHit()
    {
        if (m_CurrentState)
        {
            auto nextState = m_CurrentState->OnPepperHit(this);
            if (nextState)
            {
                TransitionTo(std::move(nextState));
            }
        }
    }

    void Enemy::OnBurgerCrush()
    {
        if (m_CurrentState)
        {
            auto nextState = m_CurrentState->OnBurgerCrush(this);
            if (nextState)
            {
                TransitionTo(std::move(nextState));
            }
        }
    }

    void Enemy::Move(float dx, float dy)
    {
        auto currentPos = GetOwner()->GetWorldPosition();
        GetOwner()->SetPosition(currentPos.x + dx, currentPos.y + dy);
    }

    void Enemy::SetSprite(const std::string& spriteName)
    {
        auto* render = GetOwner()->GetComponent<dae::RenderComponent>();
        if (render)
        {
            render->SetTexture(spriteName);
        }
    }

    glm::vec2 Enemy::GetPosition() const
    {
        auto pos = GetOwner()->GetWorldPosition();
        return glm::vec2(pos.x, pos.y);
    }

    glm::vec2 Enemy::GetPlayerPosition() const
    {
        if (m_pPlayer)
            return m_pPlayer->GetPosition();
        return glm::vec2(320.0f, 352.0f);
    }

    bool Enemy::IsOnGround() const
    {
        return GetPosition().y >= static_cast<float>(Config::WINDOW_HEIGHT) - Config::ENEMY_HEIGHT;
    }

    void Enemy::AwardPoints(int points)
    {
        std::cout << "Player earned " << points << " points!\n";
    }

    void Enemy::DestroyEnemy()
    {
        std::cout << "Enemy destroyed\n";
        MarkForRemoval();
    }
}