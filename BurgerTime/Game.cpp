#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

//#include "Minigin.h"
//#include "SceneManager.h"
//#include "ResourceManager.h"
//#include "Scene.h"
//#include "FPSComponent.h"
//#include "OrbitComponent.h"
//#include "GameObject.h"
//#include "InputManager.h"
//#include "MoveCommand.h"
//#include "RenderComponent.h"
//#include "TextComponent.h"
//#include "HealthComponent.h"
//#include "ScoreComponent.h"
//#include "LivesDisplayComponent.h"
//#include "ScoreDisplayComponent.h"
//#include "DamageCommand.h"
//#include "AddScoreCommand.h"
//#include "WinnerAchievementComponent.h"
//#include "ServiceLocator.h"
//#include "SoundIds.h"
//#include "PlaySoundCommand.h"
//
//#include "Enemy.h"
//#include "Player.h"           
//#include "PlayerState.h"      
//#include "MovementStrategy.h" 
#include "Minigin.h"
#include "SceneManager.h"
//#include "ResourceManager.h"
//#include "Scene.h"
//#include "GameObject.h"
//#include "InputManager.h"
//#include "LevelManager.h"
#include "GameConfig.h"
#include "GameManager.h"

//// Minigin Components
//#include "TextComponent.h"
//#include "RenderComponent.h"
//#include "HealthComponent.h"
//#include "ScoreComponent.h"
//#include "LivesDisplayComponent.h"
//#include "ScoreDisplayComponent.h"
//#include "FPSComponent.h"

// Minigin Commands
//#include "MoveCommand.h"
//#include "DamageCommand.h"
//#include "AddScoreCommand.h"
//#include "PlaySoundCommand.h"

// Services
//#include "ServiceLocator.h"
//#include "SoundIds.h"

// BurgerTime
//#include "Enemy.h"
//#include "Player.h"
//#include "AnimationComponent.h"
//#include "UsePepperCommand.h"

#include <filesystem>
#include "BenchmarkPi.h"
#include <iostream>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	BurgerTime::GameManager::GetInstance().Initialize(scene);
	BurgerTime::GameManager::GetInstance().ShowMainMenu();

	/*auto go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>("background.png");
	scene.Add(std::move(go));

	go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>("logo.png");
	go->SetPosition(358, 180);
	scene.Add(std::move(go));*/

	/*auto titleFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto titleGO = std::make_unique<dae::GameObject>();
	titleGO->SetPosition(292.f, 20.f);
	titleGO->AddComponent<dae::TextComponent>(
		"Programming 4 Assignment",
		titleFont,
		SDL_Color{ 255, 255, 0, 255 }
	);
	scene.Add(std::move(titleGO));*/

	// --- FPS on-screen ---
	/*auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 30);
	auto fpsGO = std::make_unique<dae::GameObject>();
	fpsGO->SetPosition(10, 8);
	fpsGO->AddComponent<dae::FPSComponent>(fpsFont, SDL_Color{ 255, 255, 255, 255 });
	scene.Add(std::move(fpsGO));*/

	// ============================================
   // LOAD LEVEL 1
   // ============================================
	
	/*BurgerTime::LevelManager::GetInstance().LoadLevel(3, scene); 

	auto infoFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);*/

	/*auto inst1GO = std::make_unique<dae::GameObject>();
	inst1GO->SetPosition(10.f, 80.f);
	inst1GO->AddComponent<dae::TextComponent>(
		"WASD = Move | C = Pepper | K = Damage | Z = Score",
		infoFont,
		SDL_Color{ 255, 255, 255, 255 }
	);
	scene.Add(std::move(inst1GO));*/

	
	/*
	auto* player1GO = BurgerTime::LevelManager::GetInstance().GetPlayer1Object();
	auto p1LivesGO = std::make_unique<dae::GameObject>();
	p1LivesGO->SetPosition(10.f, 0.f);
	p1LivesGO->AddComponent<dae::TextComponent>("", infoFont, SDL_Color{ 255, 255, 255, 255 });
	p1LivesGO->AddComponent<dae::LivesDisplayComponent>(0, 3);
	scene.Add(std::move(p1LivesGO));

	auto p1ScoreGO = std::make_unique<dae::GameObject>();
	p1ScoreGO->SetPosition(484.f, 0.f);
	p1ScoreGO->AddComponent<dae::TextComponent>("", infoFont, SDL_Color{ 255, 255, 255, 255 });
	p1ScoreGO->AddComponent<dae::ScoreDisplayComponent>(0, 0);
	scene.Add(std::move(p1ScoreGO));

	auto& input = dae::InputManager::GetInstance();
	input.ClearBindings();

	constexpr float speed = 100.0f;

	input.BindKeyboardCommand(
		SDL_SCANCODE_W,
		dae::InputState::Pressed,
		std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(player1GO, 0.f, -speed)
	);

	input.BindKeyboardCommand(
		SDL_SCANCODE_S,
		dae::InputState::Pressed,
		std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(player1GO, 0.f, speed)
	);

	input.BindKeyboardCommand(
		SDL_SCANCODE_A,
		dae::InputState::Pressed,
		std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(player1GO, -speed, 0.f)
	);

	input.BindKeyboardCommand(
		SDL_SCANCODE_D,
		dae::InputState::Pressed,
		std::make_unique<dae::ComponentMoveCommand<BurgerTime::Player>>(player1GO, speed, 0.f)
	);

	input.BindKeyboardCommand(
		SDL_SCANCODE_C,
		dae::InputState::Down,
		std::make_unique<BurgerTime::UsePepperCommand>(player1GO)
	);

	input.BindKeyboardCommand(
		SDL_SCANCODE_K,
		dae::InputState::Down,
		std::make_unique<dae::DamageCommand>(player1GO, 1)
	);

	input.BindKeyboardCommand(
		SDL_SCANCODE_Z,
		dae::InputState::Down,
		std::make_unique<dae::AddScoreCommand>(player1GO, 100)
	);



	std::cout << "=== GAME READY ===\n";
	std::cout << "WASD - Move (Component-based)\n";
	std::cout << "C - Use Pepper\n";
	std::cout << "K - Take Damage\n";
	std::cout << "Z - Add Score\n";*/
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {

	std::cout.setf(std::ios::unitbuf);
	std::cerr.setf(std::ios::unitbuf);

	std::cerr << "Program started\n";
	try
	{
	//bool benchmarkMode = false; //for benchmarks make it true. You can also pass --benchmark as a command line argument to enable benchmark mode without changing the code.
	//for (int i = 1; i < argc; ++i)
	//{
	//	if (std::string(argv[i]) == "--benchmark")
	//	{
	//		benchmarkMode = true;
	//		break;
	//	}
	//}

	//if (benchmarkMode)
	//{
	//	std::cerr << "Benchmark mode entered\n";
	//	RunPiBenchmarks();
	//	std::cerr << "Benchmark finished\n";
	//	return 0;
	//}

	//std::cerr << "Normal game mode start\n";

#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if (!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location, BurgerTime::Config::WINDOW_WIDTH, BurgerTime::Config::WINDOW_HEIGHT);
	engine.Run(load);
}
	catch (const std::runtime_error& e)
	{
		std::cerr << " RUNTIME ERROR: " << e.what() << "\n";
		return -1;
	}
	catch (const std::exception& e)
	{
		std::cerr << " EXCEPTION: " << e.what() << "\n";
		return -1;
	}
	catch (...)
	{
		std::cerr << " UNKNOWN EXCEPTION\n";
		return -1;
	}
	return 0;
}
