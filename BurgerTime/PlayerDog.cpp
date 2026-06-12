#include "PlayerDog.h"
#include "Player.h"
#include "AnimationComponent.h"
#include "GameObject.h"
#include "LevelManager.h"
#include "GameConfig.h"
#include "EngineTime.h"
#include <iostream>

namespace BurgerTime
{
    // DogNormalState
    std::unique_ptr<PlayerDogState> DogNormalState::Update(PlayerDog* dog, float)
    {
        CheckPlayerCollision(dog);
        return nullptr;
    }

    std::unique_ptr<PlayerDogState> DogNormalState::OnPepperHit(PlayerDog*)
    {
        return std::make_unique<DogStunnedState>();
    }

    std::unique_ptr<PlayerDogState> DogNormalState::OnBurgerCrush(PlayerDog*)
    {
        return std::make_unique<DogCrushedState>();
    }

    void DogNormalState::CheckPlayerCollision(PlayerDog* dog)
    {
        auto* player = dog->GetPlayerTarget();
        if (!player || !dog->CanKillPlayer()) return;

        auto dogPos = dog->GetPosition();
        auto plrPos = player->GetPosition();

        const float DW = Config::ENEMY_WIDTH;
        const float DH = Config::ENEMY_HEIGHT;
        const float PW = Config::PLAYER_WIDTH;
        const float PH = Config::PLAYER_HEIGHT;

        if (dogPos.x < plrPos.x + PW &&
            dogPos.x + DW > plrPos.x &&
            dogPos.y < plrPos.y + PH &&
            dogPos.y + DH > plrPos.y)
        {
            player->TakeDamage();
        }
    }

    // DogStunnedState
    void DogStunnedState::OnEnter(PlayerDog* dog)
    {
        m_Timer = 0.f;
        auto* anim = dog->GetOwner()->GetComponent<AnimationComponent>();
        if (anim) anim->Play("Peppered");
    }

    void DogStunnedState::OnExit(PlayerDog* dog)
    {
        auto* anim = dog->GetOwner()->GetComponent<AnimationComponent>();
        if (anim) anim->Play("Walk");
    }

    std::unique_ptr<PlayerDogState> DogStunnedState::Update(PlayerDog*, float dt)
    {
        m_Timer += dt;
        if (m_Timer >= STUN_DURATION)
            return std::make_unique<DogNormalState>();
        return nullptr;
    }

    std::unique_ptr<PlayerDogState> DogStunnedState::OnBurgerCrush(PlayerDog*)
    {
        return std::make_unique<DogCrushedState>();
    }

    // DogCrushedState
    void DogCrushedState::OnEnter(PlayerDog* dog)
    {
        m_Timer = 0.f;
        auto* anim = dog->GetOwner()->GetComponent<AnimationComponent>();
        if (anim) anim->Play("Kill");
    }

    std::unique_ptr<PlayerDogState> DogCrushedState::Update(PlayerDog* dog, float dt)
    {
        m_Timer += dt;
        if (m_Timer >= DEATH_DURATION)
        {
            dog->MarkDead();
            LevelManager::GetInstance().ClearPlayerDog();
            dog->GetOwner()->MarkForRemoval();
        }
        return nullptr;
    }

	// PlayerDog constructor and main methods
    PlayerDog::PlayerDog(dae::GameObject* owner)
        : Component(owner)
        , m_State(std::make_unique<DogNormalState>())
    {
    }

    void PlayerDog::OnAttach()
    {
        m_AnimComp = GetOwner()->GetComponent<AnimationComponent>();
        if (!m_AnimComp)
            m_AnimComp = GetOwner()->AddComponent<AnimationComponent>();

        m_AnimComp->SetRenderSize(Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT);
        SetupAnimations();

        if (m_State) m_State->OnEnter(this);
    }

    void PlayerDog::OnDetach()
    {
        LevelManager::GetInstance().ClearPlayerDog();
    }

    void PlayerDog::Update()
    {
        if (m_Dead || !IsActive() || !m_State) return;
        auto next = m_State->Update(this, dae::EngineTime::GetDeltaTime());
        if (!m_Dead && next) TransitionTo(std::move(next));
    }

    void PlayerDog::Move(float dx, float dy, float deltaTime)
    {
        if (!CanMove()) return;

        auto& lm = LevelManager::GetInstance();
        auto  cur = GetPosition();
        glm::vec2 newPos = cur;

        if (dx != 0.f && dy == 0.f)
        {
            if (!lm.IsOnPlatform(cur.x, cur.y)) return;

            newPos.x = cur.x + dx * deltaTime;
            float snapY = lm.GetNearestPlatformY(cur.x, cur.y);
            if (snapY >= 0.f) newPos.y = snapY;

            if (!lm.IsOnPlatform(newPos.x, newPos.y)) return;

            if (dx > 0.f)
            {
                const float feetY = newPos.y + Config::ENEMY_HEIGHT;
                if (!lm.IsPointOnPlatform(newPos.x + Config::ENEMY_WIDTH, feetY)) return;
            }

            m_FacingRight = (dx > 0.f);

            auto* anim = GetOwner()->GetComponent<AnimationComponent>();
            if (anim)
            {
                anim->SetFlipX(m_FacingRight);
                anim->Play("Walk");
            }
        }
        else if (dy != 0.f && dx == 0.f)
        {
            if (!lm.IsOnLadder(cur.x, cur.y)) return;

            newPos.y = cur.y + dy * deltaTime;
            float snapX = lm.GetNearestLadderX(cur.x, cur.y);
            if (snapX >= 0.f) newPos.x = snapX;

            auto* anim = GetOwner()->GetComponent<AnimationComponent>();
            if (anim) anim->Play(dy < 0.f ? "Climb" : "Descend");
        }
        else return;

        GetOwner()->SetPosition(newPos.x, newPos.y);
    }

    void PlayerDog::OnPepperHit()
    {
        if (!m_State) return;
        auto next = m_State->OnPepperHit(this);
        if (next) TransitionTo(std::move(next));
    }

    void PlayerDog::OnBurgerCrush()
    {
        if (!m_State) return;
        auto next = m_State->OnBurgerCrush(this);
        if (next) TransitionTo(std::move(next));
    }

    void PlayerDog::Freeze()
    {
        TransitionTo(std::make_unique<DogFrozenState>());
    }

    glm::vec2 PlayerDog::GetPosition() const
    {
        return GetOwner()->GetWorldPosition();
    }

    bool PlayerDog::CanKillPlayer() const
    {
        return m_State ? m_State->CanKillPlayer() : false;
    }

    bool PlayerDog::CanMove() const
    {
        return m_State ? m_State->CanMove() : false;
    }

    void PlayerDog::TransitionTo(std::unique_ptr<PlayerDogState> s)
    {
        if (!s) return;
        if (m_State) m_State->OnExit(this);
        m_State = std::move(s);
        if (m_State) m_State->OnEnter(this);
    }

    void PlayerDog::SetupAnimations()
    {
        if (!m_AnimComp) return;
        const std::string base = "BurgerTime/Enemies/Sausage_";

        auto make = [&](const char* name, const char* file, int frames, bool loop)
            {
                AnimationClip c;
                c.name = name; c.texturePath = base + file + ".png";
                c.frameCount = frames; c.rows = 1; c.columns = frames;
                c.frameTime = 0.15f; c.loop = loop; c.flipX = false;
                m_AnimComp->AddAnimation(c);
            };

        make("Walk", "Walk", 2, true);
        make("Climb", "Climb", 2, true);
        make("Descend", "Descend", 2, true);
        make("Kill", "Kill", 4, false);
        make("Peppered", "Peppered", 2, true);

        m_AnimComp->Play("Walk");
    }
}