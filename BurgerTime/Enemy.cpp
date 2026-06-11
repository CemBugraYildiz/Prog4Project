#include "Enemy.h"
#include "AnimationComponent.h"
#include "GameObject.h"
#include "Player.h"
#include "EngineTime.h"
#include "GameConfig.h"
#include "LevelManager.h"
#include "EventQueue.h"
#include "Event.h"
#include <iostream>

namespace BurgerTime
{
    void NormalState::OnEnter(Enemy* enemy)
    {
        m_Phase = Phase::Spawn;
        m_SpawnTimer = 1.0f;
        m_ClimbDir = 0.0f;
        m_TargetPlatY = -1.0f;
        m_LastPlayerSection = -1;
        m_Path.clear();
        enemy->PlayAnimation("Walk");
    }

    std::unique_ptr<EnemyState> NormalState::Update(Enemy* enemy)
    {
        auto& lm = LevelManager::GetInstance();
        auto  ep = enemy->GetPosition();
        auto  pp = enemy->GetPlayerPosition();
        float dt = dae::EngineTime::GetDeltaTime();
        bool  onPlat = lm.IsOnPlatform(ep.x, ep.y);
        bool  onLadder = lm.IsOnLadder(ep.x, ep.y);

        switch (m_Phase)
        {
        case Phase::Spawn:       
            DoSpawn(enemy, dt, onPlat, ep, pp);    
            break;
        case Phase::WalkToLadder: 
            DoWalkToLadder(enemy, dt, onPlat, ep, pp);       
            break;
        case Phase::Climbing:     
            DoClimbing(enemy, dt, onLadder, onPlat, ep);     
            break;
        case Phase::Chase:        
            DoChase(enemy, dt, onPlat, ep, pp);              
            break;
        }

        if (!onPlat && !onLadder && m_Phase != Phase::Climbing)
            enemy->Move(0.0f, 200.0f * dt);

        CheckPlayerCollision(enemy, ep);
        return nullptr;
    }

    void NormalState::DoSpawn(Enemy* enemy, float dt, bool onPlatform,
        const glm::vec2& ep, const glm::vec2& pp)
    {
        m_SpawnTimer -= dt;

        if (onPlatform)
        {
            MoveHorizontal(enemy, (pp.x > ep.x) ? 1.0f : -1.0f, dt, ep);
            enemy->PlayAnimation("Walk");
        }

        if (m_SpawnTimer <= 0.0f)
        {
            RebuildPath(ep, pp);
            m_Phase = Phase::Chase;
        }
    }

    void NormalState::DoWalkToLadder(Enemy* enemy, float dt, bool onPlatform,
        const glm::vec2& ep, const glm::vec2& pp)
    {
        if (m_Path.empty()) { m_Phase = Phase::Chase; return; }
        if (!onPlatform)    return;

        auto& lm = LevelManager::GetInstance();
        float dx = m_Path[0].ladderX - ep.x;
        float dirX = (dx > 0.0f) ? 1.0f : -1.0f;
        float newX = ep.x + dirX * SPEED * dt;

        if (std::abs(dx) < LADDER_SNAP_TOL)
        {
            const auto* destSect = lm.GetSection(m_Path[0].destSectionId);
            float destSurfaceY = destSect ? destSect->surfaceY : ep.y + Config::ENEMY_HEIGHT;

            enemy->GetOwner()->SetPosition(m_Path[0].ladderX, ep.y);
            m_ClimbDir = (destSurfaceY > ep.y + Config::ENEMY_HEIGHT) ? 1.0f : -1.0f;
            m_TargetPlatY = destSurfaceY;
            m_Path.erase(m_Path.begin());
            m_Phase = Phase::Climbing;
        }
        else if (lm.IsOnPlatform(newX, ep.y) || lm.IsOnLadder(newX, ep.y))
        {
            enemy->Move(dirX * SPEED * dt, 0.0f);
            enemy->SetFacingRight(dirX > 0.0f);
            enemy->PlayAnimation("Walk");
        }
        else
        {
            m_Path.clear();
            RebuildPath(ep, pp);
            m_Phase = Phase::Chase;
        }
    }

    void NormalState::DoClimbing(Enemy* enemy, float dt,
        bool onLadder, bool onPlatform,
        const glm::vec2& ep)
    {
        auto& lm = LevelManager::GetInstance();

        if (!onLadder)
        {
            m_ClimbDir = 0.0f;
            float snapY = lm.GetNearestPlatformY(ep.x, ep.y);
            if (snapY >= 0.0f)
                enemy->GetOwner()->SetPosition(ep.x, snapY);
            m_Phase = m_Path.empty() ? Phase::Chase : Phase::WalkToLadder;
            return;
        }

        float feetY = ep.y + Config::ENEMY_HEIGHT;

        if (onPlatform && std::abs(feetY - m_TargetPlatY) < Config::PLATFORM_SNAP_TOLERANCE * 2.0f)
        {
            float snapY = lm.GetNearestPlatformY(ep.x, ep.y);
            if (snapY >= 0.0f)
                enemy->GetOwner()->SetPosition(ep.x, snapY);
            m_ClimbDir = 0.0f;
            m_Phase = m_Path.empty() ? Phase::Chase : Phase::WalkToLadder;
            return;
        }

        enemy->Move(0.0f, m_ClimbDir * SPEED * dt);
        enemy->PlayAnimation(m_ClimbDir > 0.0f ? "Descend" : "Climb");
    }

    void NormalState::DoChase(Enemy* enemy, float dt, bool onPlatform,
        const glm::vec2& ep, const glm::vec2& pp)
    {
        auto& lm = LevelManager::GetInstance();
        int playerSect = lm.GetEntitySection(pp.x, pp.y);

        if (playerSect >= 0 && playerSect != m_LastPlayerSection)
            RebuildPath(ep, pp);

        if (!m_Path.empty()) { m_Phase = Phase::WalkToLadder; return; }

        if (!onPlatform) return;

        float dirX = (pp.x > ep.x) ? 1.0f : -1.0f;
        MoveHorizontal(enemy, dirX, dt, ep);
        enemy->PlayAnimation("Walk");
    }

    void NormalState::RebuildPath(const glm::vec2& ep, const glm::vec2& pp)
    {
        auto& lm = LevelManager::GetInstance();
        m_Path.clear();

        int enemySect = lm.GetEntitySection(ep.x, ep.y);
        int playerSect = lm.GetEntitySection(pp.x, pp.y);
        m_LastPlayerSection = playerSect;

        if (enemySect < 0 || playerSect < 0 || enemySect == playerSect) return;

        for (const auto& edge : lm.FindEnemyPath(enemySect, playerSect))
            m_Path.push_back({ edge.ladderX, edge.destSectionId });
    }

    void NormalState::MoveHorizontal(Enemy* enemy, float dirX, float dt,
        const glm::vec2& ep)
    {
        auto& lm = LevelManager::GetInstance();
        float newX = ep.x + dirX * SPEED * dt;
        if (lm.IsOnPlatform(newX, ep.y))
        {
            enemy->Move(dirX * SPEED * dt, 0.0f);
            enemy->SetFacingRight(dirX > 0.0f);
        }
    }

    void NormalState::CheckPlayerCollision(Enemy* enemy, const glm::vec2& ep)
    {
        auto* player = LevelManager::GetInstance().GetPlayer1();
        if (!player) return;

        auto pPos = player->GetPosition();
        if (
            pPos.x < ep.x + Config::ENEMY_WIDTH &&
            pPos.x + Config::PLAYER_WIDTH > ep.x &&
            pPos.y < ep.y + Config::ENEMY_HEIGHT &&
            pPos.y + Config::PLAYER_HEIGHT > ep.y)
        {
            player->TakeDamage();
        }
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
        m_StunTimer = 0.0f;
        enemy->PlayAnimation("Peppered");
    }

    void StunnedState::OnExit(Enemy*) {}

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
        return nullptr;
    }

    std::unique_ptr<EnemyState> StunnedState::OnBurgerCrush(Enemy*)
    {
        return std::make_unique<CrushedState>();
    }

    void CrushedState::OnEnter(Enemy* enemy)
    {
        enemy->PlayAnimation("Kill");
        enemy->AwardPoints(500);
    }

    std::unique_ptr<EnemyState> CrushedState::Update(Enemy* enemy)
    {
        m_DeathTimer += dae::EngineTime::GetDeltaTime();
        if (m_DeathTimer >= DEATH_DURATION)
            enemy->DestroyEnemy();
        return nullptr;
    }

    Enemy::Enemy(dae::GameObject* owner)
        : Component(owner)
        , m_CurrentState(std::make_unique<NormalState>())
    {
    }

    Enemy::~Enemy()
    {
        LevelManager::GetInstance().RemoveEnemy(this);
    }

    void Enemy::OnAttach()
    {
        m_AnimationComp = GetOwner()->AddComponent<AnimationComponent>();
        m_AnimationComp->SetRenderSize(Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT);
        SetupAnimations();

        if (m_CurrentState)
            m_CurrentState->OnEnter(this);
    }

    void Enemy::OnDetach()
    {
        if (m_CurrentState)
            m_CurrentState->OnExit(this);
    }

    void Enemy::Update()
    {
        if (!IsActive()) return;
        if (m_CurrentState)
        {
            auto nextState = m_CurrentState->Update(this);
            if (nextState)
                TransitionTo(std::move(nextState));
        }
    }

    void Enemy::Render() const {}

    void Enemy::SetupAnimations()
    {
        if (!m_AnimationComp) return;

        AnimationClip walk;
        walk.name = "Walk";
        walk.texturePath = "BurgerTime/Enemies/Egg_Walk.png";
        walk.frameCount = 2; walk.rows = 1; walk.columns = 2;
        walk.frameTime = 0.15f; walk.loop = true; walk.flipX = false;
        m_AnimationComp->AddAnimation(walk);

        AnimationClip climb;
        climb.name = "Climb";
        climb.texturePath = "BurgerTime/Enemies/Egg_Climb.png";
        climb.frameCount = 2; climb.rows = 1; climb.columns = 2;
        climb.frameTime = 0.15f; climb.loop = true; climb.flipX = false;
        m_AnimationComp->AddAnimation(climb);

        AnimationClip descend;
        descend.name = "Descend";
        descend.texturePath = "BurgerTime/Enemies/Egg_Descend.png";
        descend.frameCount = 2; descend.rows = 1; descend.columns = 2;
        descend.frameTime = 0.15f; descend.loop = true; descend.flipX = false;
        m_AnimationComp->AddAnimation(descend);

        AnimationClip kill;
        kill.name = "Kill";
        kill.texturePath = "BurgerTime/Enemies/Egg_Kill.png";
        kill.frameCount = 4; kill.rows = 1; kill.columns = 4;
        kill.frameTime = 0.15f; kill.loop = false; kill.flipX = false;
        m_AnimationComp->AddAnimation(kill);

        AnimationClip peppered;
        peppered.name = "Peppered";
        peppered.texturePath = "BurgerTime/Enemies/Egg_Peppered.png";
        peppered.frameCount = 2; peppered.rows = 1; peppered.columns = 2;
        peppered.frameTime = 0.2f; peppered.loop = true; peppered.flipX = false;
        m_AnimationComp->AddAnimation(peppered);
    }

    void Enemy::TransitionTo(std::unique_ptr<EnemyState> newState)
    {
        if (!newState) return;
        if (m_CurrentState) m_CurrentState->OnExit(this);
        m_CurrentState = std::move(newState);
        if (m_CurrentState) m_CurrentState->OnEnter(this);
    }

    void Enemy::OnPepperHit()
    {
        if (m_CurrentState)
        {
            auto nextState = m_CurrentState->OnPepperHit(this);
            if (nextState) TransitionTo(std::move(nextState));
        }
    }

    void Enemy::OnBurgerCrush()
    {
        if (m_CurrentState)
        {
            auto nextState = m_CurrentState->OnBurgerCrush(this);
            if (nextState) TransitionTo(std::move(nextState));
        }
    }

    void Enemy::Move(float dx, float dy)
    {
        auto pos = GetOwner()->GetWorldPosition();
        GetOwner()->SetPosition(pos.x + dx, pos.y + dy);
    }

    void Enemy::PlayAnimation(const std::string& name)
    {
        if (m_AnimationComp)
            m_AnimationComp->Play(name);
    }

    void Enemy::SetFacingRight(bool right)
    {
        if (m_AnimationComp)
            m_AnimationComp->SetFlipX(right);
    }

    glm::vec2 Enemy::GetPosition() const
    {
        auto pos = GetOwner()->GetWorldPosition();
        return { pos.x, pos.y };
    }

    glm::vec2 Enemy::GetPlayerPosition() const
    {
        if (m_pPlayer)
            return m_pPlayer->GetPosition();
        return { 320.0f, 352.0f };
    }

    void Enemy::AwardPoints(int points)
    {
        auto* player = LevelManager::GetInstance().GetPlayer1();
        if (player) player->AddScore(points);

        dae::Event evt;
        evt.type = dae::EventType::EnemyKilled;
        evt.value = points;
        dae::EventQueue::GetInstance().QueueEvent(evt);
    }

    void Enemy::DestroyEnemy()
    {
        GetOwner()->MarkForRemoval();
    }
}