#pragma once
#include <Component.h>
#include <glm/glm.hpp>
#include <memory>

namespace BurgerTime
{
    class Player;
    class AnimationComponent;
    class PlayerDog;

    // state interface 
    class PlayerDogState
    {
    public:
        virtual ~PlayerDogState() = default;
        virtual void OnEnter(PlayerDog*) {}
        virtual void OnExit(PlayerDog*) {}
        virtual std::unique_ptr<PlayerDogState> Update(PlayerDog*, float) { return nullptr; }
        virtual std::unique_ptr<PlayerDogState> OnPepperHit(PlayerDog*) { return nullptr; }
        virtual std::unique_ptr<PlayerDogState> OnBurgerCrush(PlayerDog*) { return nullptr; }
        virtual bool CanKillPlayer() const { return true; }
        virtual bool CanMove()       const { return true; }
    };

    // normal
    class DogNormalState final : public PlayerDogState
    {
    public:
        std::unique_ptr<PlayerDogState> Update(PlayerDog*, float) override;
        std::unique_ptr<PlayerDogState> OnPepperHit(PlayerDog*)   override;
        std::unique_ptr<PlayerDogState> OnBurgerCrush(PlayerDog*) override;
    private:
        void CheckPlayerCollision(PlayerDog*);
    };

    // stunned (0.5 s)
    class DogStunnedState final : public PlayerDogState
    {
    public:
        void OnEnter(PlayerDog*) override;
        void OnExit(PlayerDog*)  override;
        std::unique_ptr<PlayerDogState> Update(PlayerDog*, float) override;
        std::unique_ptr<PlayerDogState> OnBurgerCrush(PlayerDog*) override;
        bool CanKillPlayer() const override { return false; }
        bool CanMove()       const override { return false; }
    private:
        float m_Timer{ 0.f };
        static constexpr float STUN_DURATION = 0.5f;
    };

    // crushed 
    class DogCrushedState final : public PlayerDogState
    {
    public:
        void OnEnter(PlayerDog*) override;
        std::unique_ptr<PlayerDogState> Update(PlayerDog*, float) override;
        bool CanKillPlayer() const override { return false; }
        bool CanMove()       const override { return false; }
    private:
        float m_Timer{ 0.f };
        static constexpr float DEATH_DURATION = 0.6f;
    };

    // frozen (level complete) 
    class DogFrozenState final : public PlayerDogState
    {
        bool CanKillPlayer() const override { return false; }
        bool CanMove()       const override { return false; }
    };

    // component 
    class PlayerDog final : public dae::Component
    {
    public:
        explicit PlayerDog(dae::GameObject* owner);

        void Update() override;
        void Render() const override {}
        void OnAttach() override;
        void OnDetach() override;

        void Move(float dx, float dy, float deltaTime);
        void OnPepperHit();
        void OnBurgerCrush();
        void Freeze();

        glm::vec2 GetPosition()    const;
        bool      CanKillPlayer()  const;
        bool      CanMove()        const;

        void    SetPlayerTarget(Player* p) { m_pPlayer = p; }
        Player* GetPlayerTarget()  const { return m_pPlayer; }

        void MarkDead() { m_Dead = true; }

    private:
        std::unique_ptr<PlayerDogState> m_State;
        AnimationComponent* m_AnimComp{ nullptr };
        Player* m_pPlayer{ nullptr };
        bool m_FacingRight{ true };
        bool m_Dead{ false };

        void TransitionTo(std::unique_ptr<PlayerDogState> s);
        void SetupAnimations();
    };
}
