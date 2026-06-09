#pragma once
#include <Component.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace BurgerTime
{
    class Enemy;
    class Player;  // forward declare

    class EnemyState
    {
    public:
        virtual ~EnemyState() = default;
        virtual void OnEnter(Enemy*) {}
        virtual void OnExit(Enemy*) {}
        virtual std::unique_ptr<EnemyState> Update(Enemy*) { return nullptr; }
        virtual std::unique_ptr<EnemyState> OnPepperHit(Enemy*) { return nullptr; }
        virtual std::unique_ptr<EnemyState> OnBurgerCrush(Enemy*) { return nullptr; }
    };

    class NormalState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        std::unique_ptr<EnemyState> Update(Enemy* enemy) override;
        std::unique_ptr<EnemyState> OnPepperHit(Enemy* enemy) override;
        std::unique_ptr<EnemyState> OnBurgerCrush(Enemy* enemy) override;
    private:
        float m_Speed{ 60.0f };
    };

    class StunnedState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        void OnExit(Enemy* enemy) override;
        std::unique_ptr<EnemyState> Update(Enemy* enemy) override;
        std::unique_ptr<EnemyState> OnPepperHit(Enemy* enemy) override;
        std::unique_ptr<EnemyState> OnBurgerCrush(Enemy* enemy) override;
    private:
        float m_StunDuration{ 3.0f };
        float m_StunTimer{ 0.0f };
    };

    class CrushedState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        std::unique_ptr<EnemyState> Update(Enemy* enemy) override;
    private:
        float m_FallSpeed{ 150.0f };
    };

    class EnemyDeadState final : public EnemyState
    {
    public:
        void OnEnter(Enemy* enemy) override;
        std::unique_ptr<EnemyState> Update(Enemy* enemy) override;
    private:
        float m_DeathTimer{ 0.0f };
        float m_DeathDuration{ 1.0f };
    };

    class Enemy final : public dae::Component
    {
    public:
        explicit Enemy(dae::GameObject* owner);
        ~Enemy() override = default;

        void Update() override;
        void Render() const override;
        void OnAttach() override;
        void OnDetach() override;

        void OnPepperHit();
        void OnBurgerCrush();

        void Move(float dx, float dy);
        void SetSprite(const std::string& spriteName);
        void AwardPoints(int points);
        void DestroyEnemy();

        void SetPlayerTarget(Player* player) { m_pPlayer = player; }

        glm::vec2 GetPosition() const;
        glm::vec2 GetPlayerPosition() const;
        bool IsOnGround() const;

    private:
        std::unique_ptr<EnemyState> m_CurrentState;
        Player* m_pPlayer{ nullptr };

        void TransitionTo(std::unique_ptr<EnemyState> newState);
    };
}