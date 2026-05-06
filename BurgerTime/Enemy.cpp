#include "Enemy.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "SceneManager.h"
#include <iostream>

namespace BurgerTime
{
    // NORMAL STATE
    void NormalState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: Normal state\n";
        enemy->SetSprite("enemy_normal.png");
    }

    void NormalState::Update(Enemy* enemy)
    {
        // Simple AI - Move towards player
        auto playerPos = enemy->GetPlayerPosition();
        auto enemyPos = enemy->GetPosition();

        glm::vec2 direction = playerPos - enemyPos;
        float distance = glm::length(direction);

        if (distance > 1.0f)
        {
            direction = glm::normalize(direction);
            enemy->Move(direction.x * m_Speed, direction.y * m_Speed);
        }
    }

    void NormalState::OnPepperHit(Enemy* enemy)
    {
        enemy->ChangeState(std::make_unique<StunnedState>());
    }

    void NormalState::OnBurgerCrush(Enemy* enemy)
    {
        enemy->ChangeState(std::make_unique<CrushedState>());
    }

    // STUNNED STATE
    void StunnedState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: STUNNED!\n";
        m_StunTimer = 0;
        enemy->SetSprite("enemy_stunned.png");
    }

    void StunnedState::OnExit(Enemy*)
    {
        std::cout << "Enemy: Recovered from stun\n";
    }

    void StunnedState::Update(Enemy* enemy)
    {
        m_StunTimer++;

        // Visual feedback - could add blinking here
        // TODO: Animation

        if (m_StunTimer >= m_StunDuration)
        {
            enemy->ChangeState(std::make_unique<NormalState>());
        }
    }

    void StunnedState::OnPepperHit(Enemy*)
    {
        // Reset stun timer
        m_StunTimer = 0;
        std::cout << "Stun duration reset!\n";
    }

    void StunnedState::OnBurgerCrush(Enemy* enemy)
    {
        enemy->ChangeState(std::make_unique<CrushedState>());
    }

    // CRUSHED STATE
    void CrushedState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: CRUSHED by burger!\n";
        enemy->SetSprite("enemy_crushed.png");
        enemy->AwardPoints(500);
    }

    void CrushedState::Update(Enemy* enemy)
    {
        // Fall down with burger
        enemy->Move(0.0f, m_FallSpeed);

        // Check if hit ground
        if (enemy->IsOnGround())
        {
            enemy->ChangeState(std::make_unique<DeadState>());
        }
    }

    // DEAD STATE
    void DeadState::OnEnter(Enemy* enemy)
    {
        std::cout << "Enemy: DEAD!\n";
        enemy->SetSprite("enemy_dead.png");
    }

    void DeadState::Update(Enemy* enemy)
    {
        m_DeathTimer++;

        // Fade out animation
        // TODO: Alpha blending or scale down

        if (m_DeathTimer >= m_DeathDuration)
        {
            enemy->DestroyEnemy();
        }
    }

    // ENEMY COMPONENT
    Enemy::Enemy(dae::GameObject* owner)
        : Component(owner)
    {
        m_CurrentState = std::make_unique<NormalState>();
    }

    void Enemy::OnAttach()
    {
        // State enters when component is attached
        if (m_CurrentState)
        {
            m_CurrentState->OnEnter(this);
        }
    }

    void Enemy::OnDetach()
    {
        // State exits when component is detached
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
            m_CurrentState->Update(this);
        }
    }

    void Enemy::Render() const
    {
        // Enemy doesn't render directly
        // RenderComponent handles that
    }

    void Enemy::ChangeState(std::unique_ptr<EnemyState> newState)
    {
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
            m_CurrentState->OnPepperHit(this);
        }
    }

    void Enemy::OnBurgerCrush()
    {
        if (m_CurrentState)
        {
            m_CurrentState->OnBurgerCrush(this);
        }
    }

    // HELPER METHODS
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
        // TODO: Find player in scene
        // For now, return a test position
        return glm::vec2(200, 300);
    }

    bool Enemy::IsOnGround() const
    {
        // TODO: Platform collision check
        return GetPosition().y >= 450.0f;
    }

    void Enemy::AwardPoints(int points)
    {
        std::cout << "Player earned " << points << " points!\n";
        // TODO: Add to score component
    }

    void Enemy::DestroyEnemy()
    {
        std::cout << "Enemy destroyed\n";
        MarkForRemoval(); 
    }
}