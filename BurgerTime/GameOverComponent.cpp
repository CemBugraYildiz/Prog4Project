#include "GameOverComponent.h"
#include "GameManager.h"
#include "HighScoreManager.h"
#include "EngineTime.h"

namespace BurgerTime
{
    GameOverComponent::GameOverComponent(dae::GameObject* owner, int score)
        : Component(owner), m_Score(score) {
    }

    void GameOverComponent::Update()
    {
        if (m_Triggered) return;
        m_AutoTimer += dae::EngineTime::GetDeltaTime();
        if (m_AutoTimer >= AUTO_ADVANCE) Confirm();
    }

    void GameOverComponent::Confirm()
    {
        if (m_Triggered) return;
        m_Triggered = true;
        if (HighScoreManager::GetInstance().IsHighScore(m_Score))
            GameManager::GetInstance().ShowHighScoreEntry(m_Score);
        else
            GameManager::GetInstance().ShowMainMenu();
    }
}