#pragma once
#include <Component.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace BurgerTime
{
    class Enemy;

    // STATE INTERFACE
    class EnemyState
    {
    public:
        virtual ~EnemyState() = default;

        virtual void OnEnter(Enemy*) {}
        virtual void OnExit(Enemy*) {}
        virtual void Update(Enemy*) {}

        virtual void OnPepperHit(Enemy*) {}
        virtual void OnBurgerCrush(Enemy*) {}
    };

    // NORMAL STATE
    class NormalState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        void Update(Enemy* enemy) override;
        void OnPepperHit(Enemy* enemy) override;
        void OnBurgerCrush(Enemy* enemy) override;

    private:
        float m_Speed{ 2.0f };
    };

    // STUNNED STATE
    class StunnedState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        void OnExit(Enemy* enemy) override;
        void Update(Enemy* enemy) override;
        void OnPepperHit(Enemy* enemy) override;
        void OnBurgerCrush(Enemy* enemy) override;

    private:
        int m_StunDuration{ 180 };
        int m_StunTimer{ 0 };
    };

    // CRUSHED STATE
    class CrushedState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        void Update(Enemy* enemy) override;

    private:
        float m_FallSpeed{ 5.0f };
    };

    // DEAD STATE
    class DeadState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        void Update(Enemy* enemy) override;

    private:
        int m_DeathTimer{ 0 };
        int m_DeathDuration{ 60 };
    };

    // ENEMY COMPONENT
    class Enemy final : public dae::Component
    {
    public:
        explicit Enemy(dae::GameObject* owner);
        ~Enemy() override = default;

        void Update() override;
        void Render() const override;

        void OnAttach() override;
        void OnDetach() override;

        void ChangeState(std::unique_ptr<EnemyState> newState);

        void Move(float dx, float dy);
        void SetSprite(const std::string& spriteName);
        void AwardPoints(int points);
        void DestroyEnemy();

        glm::vec2 GetPosition() const;
        glm::vec2 GetPlayerPosition() const;
        bool IsOnGround() const;

        void OnPepperHit();
        void OnBurgerCrush();

    private:
        std::unique_ptr<EnemyState> m_CurrentState;
    };
}
