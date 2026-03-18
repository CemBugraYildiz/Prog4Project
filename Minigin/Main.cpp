#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "FPSComponent.h"
#include "OrbitComponent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "LivesDisplayComponent.h"
#include "ScoreDisplayComponent.h"
#include "DamageCommand.h"
#include "AddScoreCommand.h"
#include "WinnerAchievementComponent.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	auto go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>("background.png");
	scene.Add(std::move(go));

	go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>("logo.png");
	go->SetPosition(358, 180);
	scene.Add(std::move(go));

	auto titleFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto titleGO = std::make_unique<dae::GameObject>();
	titleGO->SetPosition(292.f, 20.f);
	titleGO->AddComponent<dae::TextComponent>(
		"Programming 4 Assignment",
		titleFont,
		SDL_Color{ 255, 255, 0, 255 }
	);
	scene.Add(std::move(titleGO));

	// --- FPS on-screen ---
	// load a small font for the FPS counter
	auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 30);
	auto fpsGO = std::make_unique<dae::GameObject>();
	fpsGO->SetPosition(10, 8); // top-left; tweak as needed
	// attach FPSComponent (owner passed automatically by AddComponent)
	fpsGO->AddComponent<dae::FPSComponent>(fpsFont, SDL_Color{ 255, 255, 255, 255 });
	scene.Add(std::move(fpsGO));

	//Instructions
	auto infoFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);

	auto inst1GO = std::make_unique<dae::GameObject>();
	inst1GO->SetPosition(10.f, 80.f);
	inst1GO->AddComponent<dae::TextComponent>(
		"Use the D-Pad to move the ladder, X to take damage, A/B to gain score",
		infoFont,
		SDL_Color{ 255, 255, 255, 255 }
	);
	scene.Add(std::move(inst1GO));

	auto inst2GO = std::make_unique<dae::GameObject>();
	inst2GO->SetPosition(10.f, 100.f);
	inst2GO->AddComponent<dae::TextComponent>(
		"Use WASD to move Peter, C to take damage, Z/X to gain score",
		infoFont,
		SDL_Color{ 255, 255, 255, 255 }
	);
	scene.Add(std::move(inst2GO));


	// Character 1
	auto player1 = std::make_unique<dae::GameObject>();
	player1->AddComponent<dae::RenderComponent>("peter.png");
	player1->SetPosition(200.f, 350.f);
	auto* player1Ptr = player1.get();
	player1->AddComponent<dae::HealthComponent>(0, 1, 3); // playerId=0, maxHealth=1, lives=3
	player1->AddComponent<dae::ScoreComponent>(0);
	scene.Add(std::move(player1));

	// Character 2
	auto player2 = std::make_unique<dae::GameObject>();
	player2->AddComponent<dae::RenderComponent>("ladder.png");
	player2->SetPosition(500.f, 350.f);
	auto* player2Ptr = player2.get();
	player2->AddComponent<dae::HealthComponent>(1, 1, 3); // playerId=1
	player2->AddComponent<dae::ScoreComponent>(1);
	scene.Add(std::move(player2));

	// -------------------------
	// UI - Player 1 Lives
	// -------------------------
	auto p1LivesGO = std::make_unique<dae::GameObject>();
	p1LivesGO->SetPosition(10.f, 140.f);
	p1LivesGO->AddComponent<dae::TextComponent>(
		"",
		infoFont,
		SDL_Color{ 255, 255, 255, 255 }
	);
	p1LivesGO->AddComponent<dae::LivesDisplayComponent>(0, 3);
	scene.Add(std::move(p1LivesGO));

	// UI - Player 1 Score
	auto p1ScoreGO = std::make_unique<dae::GameObject>();
	p1ScoreGO->SetPosition(10.f, 165.f);
	p1ScoreGO->AddComponent<dae::TextComponent>(
		"",
		infoFont,
		SDL_Color{ 255, 255, 255, 255 }
	);
	p1ScoreGO->AddComponent<dae::ScoreDisplayComponent>(0, 0);
	scene.Add(std::move(p1ScoreGO));

	// UI - Player 2 Lives
	auto p2LivesGO = std::make_unique<dae::GameObject>();
	p2LivesGO->SetPosition(10.f, 200.f);
	p2LivesGO->AddComponent<dae::TextComponent>(
		"",
		infoFont,
		SDL_Color{ 255, 255, 255, 255 }
	);
	p2LivesGO->AddComponent<dae::LivesDisplayComponent>(1, 3);
	scene.Add(std::move(p2LivesGO));

	// UI - Player 2 Score
	auto p2ScoreGO = std::make_unique<dae::GameObject>();
	p2ScoreGO->SetPosition(10.f, 225.f);
	p2ScoreGO->AddComponent<dae::TextComponent>(
		"",
		infoFont,
		SDL_Color{ 255, 255, 255, 255 }
	);
	p2ScoreGO->AddComponent<dae::ScoreDisplayComponent>(1, 0);
	scene.Add(std::move(p2ScoreGO));

	auto achievementGO = std::make_unique<dae::GameObject>();
	achievementGO->AddComponent<dae::WinnerAchievementComponent>();
	scene.Add(std::move(achievementGO));

	// -------------------------
	// Input
	// -------------------------
	auto& input = dae::InputManager::GetInstance();
	input.ClearBindings();

	constexpr float speed1 = 2.f;
	constexpr float speed2 = 4.f; // double speed

	// WASD -> player 1
	input.BindKeyboardCommand(SDL_SCANCODE_W, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, 0.f, -speed1));
	input.BindKeyboardCommand(SDL_SCANCODE_S, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, 0.f, speed1));
	input.BindKeyboardCommand(SDL_SCANCODE_A, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, -speed1, 0.f));
	input.BindKeyboardCommand(SDL_SCANCODE_D, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, speed1, 0.f));
	// Player 1 - test death / score
	input.BindKeyboardCommand(SDL_SCANCODE_C, dae::InputState::Down, std::make_unique<dae::DamageCommand>(player1Ptr, 1));
	input.BindKeyboardCommand(SDL_SCANCODE_Z, dae::InputState::Down, std::make_unique<dae::AddScoreCommand>(player1Ptr, 100));
	input.BindKeyboardCommand(SDL_SCANCODE_X, dae::InputState::Down, std::make_unique<dae::AddScoreCommand>(player1Ptr, 200));

	// Controller 0 DPad -> player 2
	input.BindControllerCommand(0, dae::ControllerButton::DPadUp, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, 0.f, -speed2));
	input.BindControllerCommand(0, dae::ControllerButton::DPadDown, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, 0.f, speed2));
	input.BindControllerCommand(0, dae::ControllerButton::DPadLeft, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, -speed2, 0.f));
	input.BindControllerCommand(0, dae::ControllerButton::DPadRight, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, speed2, 0.f));
	// Player 2 - test death / score
	input.BindControllerCommand(0, dae::ControllerButton::X, dae::InputState::Down, std::make_unique<dae::DamageCommand>(player2Ptr, 1));
	input.BindControllerCommand(0, dae::ControllerButton::A, dae::InputState::Down, std::make_unique<dae::AddScoreCommand>(player2Ptr, 100));
	input.BindControllerCommand(0, dae::ControllerButton::B, dae::InputState::Down, std::make_unique<dae::AddScoreCommand>(player2Ptr, 200));
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
    return 0;
}
