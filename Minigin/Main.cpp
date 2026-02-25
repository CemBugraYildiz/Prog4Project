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

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	auto go = std::make_unique<dae::GameObject>();
	go->SetTexture("background.png");
	scene.Add(std::move(go));

	go = std::make_unique<dae::GameObject>();
	go->SetTexture("logo.png");
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

	// --- Orbit demonstration ---
	// Parent object orbits around a fixed center on screen
	auto parent = std::make_unique<dae::GameObject>();
	parent->SetTexture("peter.png");
	// Orbit center roughly center-top area (tweak to taste)
	parent->AddComponent<dae::OrbitComponent>(150.0f /*radius*/, 0.8f /*rad/s*/, false /*useParentAsCenter*/, glm::vec2{ 512.0f, 200.0f }, 0.0f);
	// Add to scene as root
	auto parentPtr = parent.get();
	scene.Add(std::move(parent));

	// Child object that orbits around parent (attached as child) — ladder.png
	auto child = std::make_unique<dae::GameObject>();
	child->SetTexture("ladder.png");
	// initial local position relative to parent
	child->SetPosition(60.0f, 0.0f);
	// orbit around parent with a different speed and radius
	child->AddComponent<dae::OrbitComponent>(60.0f /*radius*/, 2.5f /*rad/s*/, true /*useParentAsCenter*/, glm::vec2{ 0,0 }, 0.0f);

	// Attach child to parent:
	parentPtr->AttachChild(std::move(child));
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
