#pragma once
#include <Component.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace BurgerTime
{
    class Enemy;
    class Player;
    class AnimationComponent;

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
        enum class Phase { Spawn, WalkToLadder, Climbing, Chase };

        struct PathStep { float ladderX; int destSectionId; };

        static constexpr float SPEED = 60.0f;
        static constexpr float LADDER_SNAP_TOL = 3.0f;

        Phase m_Phase{ Phase::Spawn };

        float m_SpawnTimer{ 1.0f };
        float m_ClimbDir{ 0.0f };
        float m_TargetPlatY{ -1.0f }; 
        int m_LastPlayerSection{ -1 };

        std::vector<PathStep> m_Path;

        void DoSpawn(Enemy* e, float dt, bool onPlatform,
            const glm::vec2& ep, const glm::vec2& pp);
        void DoWalkToLadder(Enemy* e, float dt, bool onPlatform,
            const glm::vec2& ep, const glm::vec2& pp);
        void DoClimbing(Enemy* e, float dt, bool onLadder, bool onPlatform,
            const glm::vec2& ep);
        void DoChase(Enemy* e, float dt, bool onPlatform,
            const glm::vec2& ep, const glm::vec2& pp);

        void RebuildPath(const glm::vec2& ep, const glm::vec2& pp);
        void MoveHorizontal(Enemy* e, float dirX, float dt, const glm::vec2& ep);
        void CheckPlayerCollision(Enemy* e, const glm::vec2& ep);
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
        float m_DeathTimer{ 0.0f };
        static constexpr float DEATH_DURATION = 0.6f;
    };

    class Enemy final : public dae::Component
    {
    public:
        explicit Enemy(dae::GameObject* owner);
        ~Enemy() override;

        void Update() override;
        void Render() const override;
        void OnAttach() override;
        void OnDetach() override;

        void OnPepperHit();
        void OnBurgerCrush();

        void Move(float dx, float dy);
        void PlayAnimation(const std::string& name);
        void SetFacingRight(bool right);
        void AwardPoints(int points);
        void DestroyEnemy();

        void SetPlayerTarget(Player* player) { m_pPlayer = player; }

        glm::vec2 GetPosition() const;
        glm::vec2 GetPlayerPosition() const;

    private:
        std::unique_ptr<EnemyState> m_CurrentState;
        Player* m_pPlayer{ nullptr };
        AnimationComponent* m_AnimationComp{ nullptr };

        void TransitionTo(std::unique_ptr<EnemyState> newState);
        void SetupAnimations();
    };
}