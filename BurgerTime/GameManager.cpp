#include "GameManager.h"
#include "MainMenuComponent.h"
#include "GameOverComponent.h"
#include "HighScoreEntryComponent.h"
#include "HighScoreManager.h"
#include "LevelManager.h"
#include "FuncCommand.h"
#include "PepperDisplayComponent.h"
#include "LivesDisplayComponent.h"
#include "ScoreDisplayComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "ServiceLocator.h"
#include "SoundIds.h"
#include "EventQueue.h"
#include "Event.h"
#include "ScoreComponent.h"
#include "MoveCommand.h" 
#include "UsePepperCommand.h"

#include <fstream>
#include <string>
#include <iostream>

namespace BurgerTime
{
    void GameManager::Initialize(dae::Scene& scene)
    {
        m_pScene = &scene;
        HighScoreManager::GetInstance().Load();
        dae::EventQueue::GetInstance().AddListener(dae::EventType::GameOver, this);
        dae::EventQueue::GetInstance().AddListener(dae::EventType::RequestNextLevel, this);
    }

    void GameManager::OnEvent(const dae::Event& event)
    {
        if (event.type == dae::EventType::RequestNextLevel)
            LoadNextLevel();
        else if (event.type == dae::EventType::GameOver)
            ShowGameOver(event.value);
    }

    void GameManager::ShowMainMenu()
    {
        m_CurrentLevel = 1;
        ClearScene();
        dae::ServiceLocator::GetSoundSystem().StopMusic();
        BuildMainMenuScene();
    }

    void GameManager::StartGame(int levelId)
    {
        m_CurrentLevel = levelId;
        m_CurrentScreen = GameScreen::Gameplay;
        ClearScene();
        LevelManager::GetInstance().LoadLevel(levelId, *m_pScene);
        BuildHUD();
        BindGameplayKeys();
        dae::ServiceLocator::GetSoundSystem().PlayMusic(dae::MUSIC_GAMEPLAY, 0.5f, true);
    }

    void GameManager::LoadNextLevel()
    {
        int next = m_CurrentLevel + 1;
        std::string path = "Data/BurgerTime/Levels/level" + std::to_string(next) + ".json";
        std::ifstream test(path);
        if (!test.good()) { ShowMainMenu(); return; }
        StartGame(next);
    }

    void GameManager::ShowGameOver(int score)
    {
        m_CurrentScreen = GameScreen::GameOver;
        ClearScene();
        dae::ServiceLocator::GetSoundSystem().StopMusic();
        BuildGameOverScene(score);
    }

    void GameManager::ShowHighScoreEntry(int score)
    {
        m_CurrentScreen = GameScreen::HighScoreEntry;
        ClearScene();
        BuildHighScoreEntryScene(score);
    }

    void GameManager::ShowHighScoreDisplay()
    {
        m_CurrentScreen = GameScreen::HighScoreDisplay;
        ClearScene();
        BuildHighScoreDisplayScene();
    }

    void GameManager::ToggleSound()
    {
        m_SoundEnabled = !m_SoundEnabled;
        dae::ServiceLocator::GetSoundSystem().SetMuted(!m_SoundEnabled);

        if (m_SoundEnabled && m_CurrentScreen == GameScreen::Gameplay)
            dae::ServiceLocator::GetSoundSystem().PlayMusic(dae::MUSIC_GAMEPLAY, 0.5f, true);
    }

    void GameManager::ClearScene()
    {
        if (LevelManager::GetInstance().GetCurrentLevelId() > 0)
            LevelManager::GetInstance().UnloadCurrentLevel(*m_pScene);
        else
            m_pScene->Clear();
        dae::InputManager::GetInstance().ClearBindings();
    }

    void GameManager::BuildMainMenuScene()
    {
        m_CurrentScreen = GameScreen::MainMenu;
        auto& rm = dae::ResourceManager::GetInstance();

        // Title
        auto titleFont = rm.LoadFont("BurgerTime/Burgertime.otf", 60);
        auto titleGO = std::make_unique<dae::GameObject>();
        titleGO->SetPosition(80.f, 210.f);
        titleGO->AddComponent<dae::TextComponent>("BurgerTime", titleFont,
            SDL_Color{ 255, 200, 0, 255 });
        m_pScene->Add(std::move(titleGO));

        // Menu handler component
        auto handlerGO = std::make_unique<dae::GameObject>();
        auto* menuComp = handlerGO->AddComponent<MainMenuComponent>();
        m_pScene->Add(std::move(handlerGO));

        // Option fonts
        auto normalFont = rm.LoadFont("Lingua.otf", 28);
        auto selectedFont = rm.LoadFont("Lingua.otf", 36);

        const char* names[] = { "Single Player", "Co-op", "Versus" };
        const float  ys[] = { 370.f, 435.f, 500.f };
        const float  xNorm = 195.f;
        const float  xSel = 178.f;

        for (int i = 0; i < 3; ++i)
        {
            // Normal (inactive when selected)
            auto nGO = std::make_unique<dae::GameObject>();
            nGO->SetPosition(xNorm, ys[i]);
            auto* nTC = nGO->AddComponent<dae::TextComponent>(
                names[i], normalFont, SDL_Color{ 200, 200, 200, 255 });
            m_pScene->Add(std::move(nGO));

            // Selected (active when selected)
            auto sGO = std::make_unique<dae::GameObject>();
            sGO->SetPosition(xSel, ys[i] - 4.f);
            auto* sTC = sGO->AddComponent<dae::TextComponent>(
                names[i], selectedFont, SDL_Color{ 0, 255, 100, 255 });
            m_pScene->Add(std::move(sGO));

            menuComp->AddOption(nTC, sTC);
        }
        menuComp->UpdateVisuals();
        BindMainMenuKeys(menuComp);
    }

    void GameManager::BuildGameOverScene(int score)
    {
        auto& rm = dae::ResourceManager::GetInstance();
        auto  big = rm.LoadFont("Lingua.otf", 48);
        auto  med = rm.LoadFont("Lingua.otf", 30);
        auto  small = rm.LoadFont("Lingua.otf", 22);

        auto goGO = std::make_unique<dae::GameObject>();
        goGO->SetPosition(190.f, 260.f);
        goGO->AddComponent<dae::TextComponent>("GAME OVER", big, SDL_Color{ 255, 60, 60, 255 });
        m_pScene->Add(std::move(goGO));

        auto scGO = std::make_unique<dae::GameObject>();
        scGO->SetPosition(220.f, 340.f);
        scGO->AddComponent<dae::TextComponent>("Score: " + std::to_string(score), med,
            SDL_Color{ 255, 255, 255, 255 });
        m_pScene->Add(std::move(scGO));

        auto hintGO = std::make_unique<dae::GameObject>();
        hintGO->SetPosition(155.f, 410.f);
        hintGO->AddComponent<dae::TextComponent>("Press Enter to continue", small,
            SDL_Color{ 180, 180, 180, 255 });
        m_pScene->Add(std::move(hintGO));

        // GameOverComponent handles confirm input
        auto compGO = std::make_unique<dae::GameObject>();
        auto* goComp = compGO->AddComponent<GameOverComponent>(score);
        m_pScene->Add(std::move(compGO));

        // Bind confirm
        auto& input = dae::InputManager::GetInstance();
        input.BindKeyboardCommand(SDL_SCANCODE_RETURN, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([goComp] { goComp->Confirm(); }));
        input.BindControllerCommand(0, dae::ControllerButton::A, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([goComp] { goComp->Confirm(); }));
    }

    void GameManager::BuildHighScoreEntryScene(int score)
    {
        auto& rm = dae::ResourceManager::GetInstance();
        auto  hdr = rm.LoadFont("Lingua.otf", 30);
        auto  med = rm.LoadFont("Lingua.otf", 26);
        auto  whl = rm.LoadFont("Lingua.otf", 22);

        auto hdrGO = std::make_unique<dae::GameObject>();
        hdrGO->SetPosition(170.f, 200.f);
        hdrGO->AddComponent<dae::TextComponent>("ENTER YOUR NAME", hdr,
            SDL_Color{ 255, 200, 0, 255 });
        m_pScene->Add(std::move(hdrGO));

        // Name display (updated dynamically)
        auto namGO = std::make_unique<dae::GameObject>();
        namGO->SetPosition(240.f, 270.f);
        auto* namTC = namGO->AddComponent<dae::TextComponent>("_ _ _", med,
            SDL_Color{ 255, 255, 255, 255 });
        m_pScene->Add(std::move(namGO));

        // Wheel display (updated dynamically)
        auto whlGO = std::make_unique<dae::GameObject>();
        whlGO->SetPosition(80.f, 340.f);
        auto* whlTC = whlGO->AddComponent<dae::TextComponent>("", whl,
            SDL_Color{ 200, 200, 200, 255 });
        m_pScene->Add(std::move(whlGO));

        // Hint
        auto hntGO = std::make_unique<dae::GameObject>();
        hntGO->SetPosition(120.f, 410.f);
        hntGO->AddComponent<dae::TextComponent>(
            "Left/Right: select  Enter/A: confirm  BS/B: delete", whl,
            SDL_Color{ 140, 140, 140, 255 });
        m_pScene->Add(std::move(hntGO));

        auto compGO = std::make_unique<dae::GameObject>();
        auto* hsComp = compGO->AddComponent<HighScoreEntryComponent>(score, namTC, whlTC);
        m_pScene->Add(std::move(compGO));

        hsComp->RefreshDisplay();

        auto& input = dae::InputManager::GetInstance();
        input.BindKeyboardCommand(SDL_SCANCODE_LEFT, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->MoveLetter(-1); }));
        input.BindKeyboardCommand(SDL_SCANCODE_RIGHT, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->MoveLetter(1); }));
        input.BindKeyboardCommand(SDL_SCANCODE_RETURN, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->ConfirmLetter(); }));
        input.BindKeyboardCommand(SDL_SCANCODE_BACKSPACE, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->DeleteLetter(); }));
        input.BindControllerCommand(0, dae::ControllerButton::DPadLeft, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->MoveLetter(-1); }));
        input.BindControllerCommand(0, dae::ControllerButton::DPadRight, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->MoveLetter(1); }));
        input.BindControllerCommand(0, dae::ControllerButton::A, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->ConfirmLetter(); }));
        input.BindControllerCommand(0, dae::ControllerButton::B, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([hsComp] { hsComp->DeleteLetter(); }));
    }

    void GameManager::BuildHighScoreDisplayScene()
    {
        auto& rm = dae::ResourceManager::GetInstance();
        auto  hdr = rm.LoadFont("Lingua.otf", 32);
        auto  ent = rm.LoadFont("Lingua.otf", 24);
        auto  sml = rm.LoadFont("Lingua.otf", 20);

        auto hdrGO = std::make_unique<dae::GameObject>();
        hdrGO->SetPosition(210.f, 150.f);
        hdrGO->AddComponent<dae::TextComponent>("HIGH SCORES", hdr,
            SDL_Color{ 255, 200, 0, 255 });
        m_pScene->Add(std::move(hdrGO));

        const auto& entries = HighScoreManager::GetInstance().GetEntries();
        float y = 210.f;
        int   rank = 1;
        for (const auto& e : entries)
        {
            std::string line = std::to_string(rank) + ".  " + e.name + "    " + std::to_string(e.score);
            auto go = std::make_unique<dae::GameObject>();
            go->SetPosition(180.f, y);
            go->AddComponent<dae::TextComponent>(line, ent, SDL_Color{ 255, 255, 255, 255 });
            m_pScene->Add(std::move(go));
            y += 36.f; ++rank;
        }

        auto hntGO = std::make_unique<dae::GameObject>();
        hntGO->SetPosition(180.f, y + 20.f);
        hntGO->AddComponent<dae::TextComponent>("Press Enter to return to menu", sml,
            SDL_Color{ 140, 140, 140, 255 });
        m_pScene->Add(std::move(hntGO));

        // Confirm main menu
        auto compGO = std::make_unique<dae::GameObject>();
        // Reuse a minimal wrapper using FuncCommand
        m_pScene->Add(std::move(compGO));

        auto& input = dae::InputManager::GetInstance();
        input.BindKeyboardCommand(SDL_SCANCODE_RETURN, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([] { GameManager::GetInstance().ShowMainMenu(); }));
        input.BindControllerCommand(0, dae::ControllerButton::A, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([] { GameManager::GetInstance().ShowMainMenu(); }));
    }

    void GameManager::BuildHUD()
    {
        auto& rm = dae::ResourceManager::GetInstance();
        auto  font = rm.LoadFont("Lingua.otf", 22);

        auto livesGO = std::make_unique<dae::GameObject>();
        livesGO->SetPosition(10.f, 55.f);
        livesGO->AddComponent<dae::TextComponent>("", font, SDL_Color{ 255, 255, 255, 255 });
        livesGO->AddComponent<dae::LivesDisplayComponent>(0, 3);
        m_pScene->Add(std::move(livesGO));

        auto scoreGO = std::make_unique<dae::GameObject>();
        scoreGO->SetPosition(10.f, 80.f);
        scoreGO->AddComponent<dae::TextComponent>("", font, SDL_Color{ 255, 255, 255, 255 });
        scoreGO->AddComponent<dae::ScoreDisplayComponent>(0, 0);
        m_pScene->Add(std::move(scoreGO));

        auto pepperGO = std::make_unique<dae::GameObject>();
        pepperGO->SetPosition(10.f, 105.f);
        pepperGO->AddComponent<dae::TextComponent>("Pepper: 5", font, SDL_Color{ 255, 255, 100, 255 });
        pepperGO->AddComponent<BurgerTime::PepperDisplayComponent>(0);
        m_pScene->Add(std::move(pepperGO));
    }

    void GameManager::BindMainMenuKeys(MainMenuComponent* menu)
    {
        auto& input = dae::InputManager::GetInstance();

        input.BindKeyboardCommand(SDL_SCANCODE_UP, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Navigate(-1); }));
        input.BindKeyboardCommand(SDL_SCANCODE_DOWN, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Navigate(1); }));
        input.BindKeyboardCommand(SDL_SCANCODE_W, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Navigate(-1); }));
        input.BindKeyboardCommand(SDL_SCANCODE_S, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Navigate(1); }));
        input.BindKeyboardCommand(SDL_SCANCODE_RETURN, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Confirm(); }));
        input.BindKeyboardCommand(SDL_SCANCODE_SPACE, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Confirm(); }));

        input.BindControllerCommand(0, dae::ControllerButton::DPadUp, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Navigate(-1); }));
        input.BindControllerCommand(0, dae::ControllerButton::DPadDown, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Navigate(1); }));
        input.BindControllerCommand(0, dae::ControllerButton::A, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([menu] { menu->Confirm(); }));
    }

    void GameManager::BindGameplayKeys()
    {
        auto& input = dae::InputManager::GetInstance();
        auto* p1GO = LevelManager::GetInstance().GetPlayer1Object();
        if (!p1GO) return;

        constexpr float spd = 100.f;

        input.BindKeyboardCommand(SDL_SCANCODE_W, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, 0.f, -spd));
        input.BindKeyboardCommand(SDL_SCANCODE_S, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, 0.f, spd));
        input.BindKeyboardCommand(SDL_SCANCODE_A, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, -spd, 0.f));
        input.BindKeyboardCommand(SDL_SCANCODE_D, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, spd, 0.f));

        input.BindKeyboardCommand(SDL_SCANCODE_C, dae::InputState::Down,
            std::make_unique<BurgerTime::UsePepperCommand>(p1GO));

        //  Gamepad (controller 0)
        input.BindControllerCommand(0, dae::ControllerButton::DPadUp, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, 0.f, -spd));
        input.BindControllerCommand(0, dae::ControllerButton::DPadDown, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, 0.f, spd));
        input.BindControllerCommand(0, dae::ControllerButton::DPadLeft, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, -spd, 0.f));
        input.BindControllerCommand(0, dae::ControllerButton::DPadRight, dae::InputState::Pressed,
            std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(p1GO, spd, 0.f));
        input.BindControllerCommand(0, dae::ControllerButton::A, dae::InputState::Down,
            std::make_unique<BurgerTime::UsePepperCommand>(p1GO));

        // F1  next level
        input.BindKeyboardCommand(SDL_SCANCODE_F1, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([] { GameManager::GetInstance().LoadNextLevel(); }));
        // F2  toggle sound
        input.BindKeyboardCommand(SDL_SCANCODE_F2, dae::InputState::Down,
            std::make_unique<dae::FuncCommand>([] { GameManager::GetInstance().ToggleSound(); }));
    }
}