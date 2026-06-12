#pragma once
#include <memory>

namespace BurgerTime
{
    class Player;  // Forward declaration

    // PLAYER STATE INTERFACE
    class PlayerState
    {
    public:
        virtual ~PlayerState() = default;

        virtual void OnEnter(Player*) {}
        virtual void OnExit(Player*) {}
        virtual std::unique_ptr<PlayerState> Update(Player*, float deltaTime) { return nullptr; }
        virtual std::unique_ptr<PlayerState> HandleInput(Player*, float dx, float dy) { return nullptr; }

        virtual bool CanMove() const { return true; }
        virtual bool CanUsePepper() const { return true; }
        virtual bool CanClimb() const { return false; }
    };

    // IDLE STATE
    class PlayerIdleState final : public PlayerState
    {
    public:
        void OnEnter(Player* player) override;
        std::unique_ptr<PlayerState> HandleInput(Player* player, float dx, float dy) override;
    };

    // WALKING STATE
    class PlayerWalkingState final : public PlayerState
    {
    public:
        void OnEnter(Player* player) override;
        void OnExit(Player* player) override;
        std::unique_ptr<PlayerState> Update(Player* player, float deltaTime) override;
        std::unique_ptr<PlayerState> HandleInput(Player* player, float dx, float dy) override;

    private:
        float m_WalkTimer{ 0.0f };
    };

    // CLIMBING STATE
    class PlayerClimbingState final : public PlayerState
    {
    public:
        void OnEnter(Player* player) override;
        void OnExit(Player* player) override;
        std::unique_ptr<PlayerState> Update(Player* player, float deltaTime) override;
        std::unique_ptr<PlayerState> HandleInput(Player* player, float dx, float dy) override;

        bool CanClimb() const override { return true; }
    };

    // DEAD STATE
    class PlayerDeadState final : public PlayerState
    {
    public:
        void OnEnter(Player* player) override;
        std::unique_ptr<PlayerState> Update(Player* player, float deltaTime) override;

        bool CanMove() const override { return false; }
        bool CanUsePepper() const override { return false; }

    private:
        float m_DeathTimer{ 0.0f };
        const float m_DeathDuration{ 2.0f };
    };

    class PlayerVictoryState final : public PlayerState
    {
    public:
        void OnEnter(Player* player) override;
        std::unique_ptr<PlayerState> Update(Player* player, float deltaTime) override;
        bool CanMove()      const override { return false; }
        bool CanUsePepper() const override { return false; }

    private:
        float m_Timer{ 0.f };
        bool  m_Fired{ false };
        static constexpr float DURATION = 3.0f;
    };
}