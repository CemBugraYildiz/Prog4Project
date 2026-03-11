#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextObject.h"
#include "Scene.h"
#include "FPSComponent.h"
#include "OrbitComponent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "RenderComponent.h"
#include "TextComponent.h"

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

	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto to = std::make_unique<dae::TextObject>("Programming 4 Assignment", font);
	to->SetColor({ 255, 255, 0, 255 });
	to->SetPosition(292, 20);
	scene.Add(std::move(to));

	// --- FPS on-screen ---
	// load a small font for the FPS counter
	auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 30);
	auto fpsGO = std::make_unique<dae::GameObject>();
	fpsGO->SetPosition(10, 8); // top-left; tweak as needed
	// attach FPSComponent (owner passed automatically by AddComponent)
	fpsGO->AddComponent<dae::FPSComponent>(fpsFont, SDL_Color{ 255, 255, 255, 255 });
	scene.Add(std::move(fpsGO));

	//Instructions
	auto inst1 = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);
	auto inst1To = std::make_unique<dae::TextObject>("Use the D-Pad to move the ladder", inst1
	);
	inst1To->SetColor({ 255, 255, 255, 255 });
	inst1To->SetPosition(10, 80);
	scene.Add(std::move(inst1To));

	auto inst2 = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);
	auto inst2To = std::make_unique<dae::TextObject>("Use the WASD to move Peter", inst2);
	inst2To->SetColor({ 255, 255, 255, 255 });
	inst2To->SetPosition(10, 100);
	scene.Add(std::move(inst2To));

	// Character 1
	auto player1 = std::make_unique<dae::GameObject>();
	player1->AddComponent<dae::RenderComponent>("peter.png");
	player1->SetPosition(200.f, 350.f);
	auto* player1Ptr = player1.get();
	scene.Add(std::move(player1));

	// Character 2
	auto player2 = std::make_unique<dae::GameObject>();
	player2->AddComponent<dae::RenderComponent>("ladder.png");
	player2->SetPosition(500.f, 350.f);
	auto* player2Ptr = player2.get();
	scene.Add(std::move(player2));

	auto& input = dae::InputManager::GetInstance();
	input.ClearBindings();

	constexpr float speed1 = 2.f;
	constexpr float speed2 = 4.f; // double speed

	// WASD -> player 1
	input.BindKeyboardCommand(SDL_SCANCODE_W, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, 0.f, -speed1));
	input.BindKeyboardCommand(SDL_SCANCODE_S, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, 0.f, speed1));
	input.BindKeyboardCommand(SDL_SCANCODE_A, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, -speed1, 0.f));
	input.BindKeyboardCommand(SDL_SCANCODE_D, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player1Ptr, speed1, 0.f));

	// Controller 0 DPad -> player 2
	input.BindControllerCommand(0, dae::ControllerButton::DPadUp, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, 0.f, -speed2));
	input.BindControllerCommand(0, dae::ControllerButton::DPadDown, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, 0.f, speed2));
	input.BindControllerCommand(0, dae::ControllerButton::DPadLeft, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, -speed2, 0.f));
	input.BindControllerCommand(0, dae::ControllerButton::DPadRight, dae::InputState::Pressed, std::make_unique<dae::MoveCommand>(player2Ptr, speed2, 0.f));
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
