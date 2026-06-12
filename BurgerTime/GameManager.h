#pragma once
#include "Singleton.h"
#include "IEventListener.h"
#include "GameMode.h" 

namespace dae { class Scene; }

namespace BurgerTime
{
    class MainMenuComponent;
    class GameOverComponent;
    class HighScoreEntryComponent;

    enum class GameScreen { MainMenu, Gameplay, GameOver, HighScoreEntry, HighScoreDisplay };

    class GameManager final : public dae::Singleton<GameManager>
        , public dae::IEventListener
    {
    public:
        void Initialize(dae::Scene& scene);
        void OnEvent(const dae::Event& event) override;

        void ShowMainMenu();
        void StartGame(int levelId = 1, GameMode mode = GameMode::SinglePlayer);
        void LoadNextLevel();
        void ShowGameOver(int score);
        void ShowHighScoreEntry(int score);
        void ShowHighScoreDisplay();

        void ToggleSound();
        bool IsSoundEnabled() const { return m_SoundEnabled; }
        int  GetCurrentLevel() const { return m_CurrentLevel; }

        GameMode GetGameMode() const { return m_GameMode; }

    private:
        friend class dae::Singleton<GameManager>;
        GameManager() = default;

        void BuildMainMenuScene();
        void BuildGameOverScene(int score);
        void BuildHighScoreEntryScene(int score);
        void BuildHighScoreDisplayScene();
        void BuildHUD();
        void BindMainMenuKeys(MainMenuComponent* menu);
        void BindGameplayKeys();
        void ClearScene();

        dae::Scene* m_pScene{ nullptr };
        GameScreen   m_CurrentScreen{ GameScreen::MainMenu };
        int          m_CurrentLevel{ 1 };
        GameMode m_GameMode{ GameMode::SinglePlayer };
        bool         m_SoundEnabled{ true };
    };
}
