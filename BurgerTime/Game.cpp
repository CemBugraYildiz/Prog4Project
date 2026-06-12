#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "GameConfig.h"
#include "GameManager.h"
//#include "FPSComponent.h"

#include <filesystem>
#include "BenchmarkPi.h"
#include <iostream>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	BurgerTime::GameManager::GetInstance().Initialize(scene);
	BurgerTime::GameManager::GetInstance().ShowMainMenu();

	// --- FPS on-screen ---
	/*auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 30);
	auto fpsGO = std::make_unique<dae::GameObject>();
	fpsGO->SetPosition(10, 8);
	fpsGO->AddComponent<dae::FPSComponent>(fpsFont, SDL_Color{ 255, 255, 255, 255 });
	scene.Add(std::move(fpsGO));*/

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
