#pragma once
#include <Component.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace BurgerTime
{
    class PlayerState;
    class CollisionManager;
    class AnimationComponent;

    enum class Direction
    {
        None,
        Up,
        Down,
        Left,
        Right
    };

    class Player final : public dae::Component
    {
    public:
        explicit Player(dae::GameObject* owner, int playerId);
        ~Player() override;

        // Rule of Five
        Player(const Player&) = delete;
        Player& operator=(const Player&) = delete;
        Player(Player&&) = delete;
        Player& operator=(Player&&) = delete;

        void Update() override;
        void Render() const override;
        void OnAttach() override;
        void OnDetach() override;

        // ============================================
        // PUBLIC API
        // ============================================
        void Move(float dx, float dy, float deltaTime);
        void UsePepper();
        void TakeDamage();
        void AddScore(int points);
        void Reset();

        // Getters
        int GetPlayerId() const { return m_PlayerId; }
        glm::vec2 GetPosition() const;
        Direction GetFacingDirection() const { return m_FacingDirection; }
        int GetPepperCount() const { return m_PepperCount; }
        bool IsAlive() const;
        bool CanMove() const;
        bool CanUsePepper() const;

        // Collision
        bool IsOnPlatform() const;
        bool IsOnLadder() const;
        bool CanMoveTo(const glm::vec2& position) const;
        void SetCollisionManager(CollisionManager* manager) { m_CollisionManager = manager; }
        CollisionManager* GetCollisionManager() const { return m_CollisionManager; }

        // State management
        void TransitionTo(std::unique_ptr<PlayerState> newState);
        PlayerState* GetCurrentState() const { return m_CurrentState.get(); }

        // Animation
        void PlayAnimation(const std::string& animName);
        void SetFacingDirection(Direction dir);
        AnimationComponent* GetAnimationComponent() const { return m_AnimationComp; }

        void TransitionToVictory();

    private:
        // ============================================
        // MEMBER VARIABLES
        // ============================================
        int m_PlayerId;

        // State
        std::unique_ptr<PlayerState> m_CurrentState;

        // Movement
        Direction m_FacingDirection{ Direction::Right };

        // Gameplay
        int m_PepperCount{ 5 };
        glm::vec2 m_SpawnPosition{ 0.0f, 0.0f };
        bool  m_IsInvincible{ false };
        float m_InvincibleTimer{ 0.0f };
        static constexpr float INVINCIBLE_DURATION = 1.5f;

        // External systems
        CollisionManager* m_CollisionManager{ nullptr };
        AnimationComponent* m_AnimationComp{ nullptr };

        glm::vec2 m_LastPosition{ 0.0f, 0.0f };

        // HELPER METHODS
        void SetupAnimations();
    };
}