#include <stdexcept>
#include <sstream>
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>

#if WIN32
// Avoid pollution from windows.h (min/max macros etc.)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef USE_STEAMWORKS
#pragma warning(push)
#pragma warning(disable:4996)
#include <steam_api.h>
#pragma warning(pop)
#endif

#include <SDL3/SDL.h>
//#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Minigin.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "EventQueue.h"

SDL_Window* g_window{};

void LogSDLVersion(const std::string& message, int major, int minor, int patch)
{
#if WIN32
	std::stringstream ss;
	ss << message << major << "." << minor << "." << patch << "\n";
	OutputDebugString(ss.str().c_str());
#else
	std::cout << message << major << "." << minor << "." << patch << "\n";
#endif
}

#ifdef __EMSCRIPTEN__
#include "emscripten.h"

void LoopCallback(void* arg)
{
	static_cast<dae::Minigin*>(arg)->RunOneFrame();
}
#endif

// Why bother with this? Because sometimes students have a different SDL version installed on their pc.
// That is not a problem unless for some reason the dll's from this project are not copied next to the exe.
// These entries in the debug output help to identify that issue.
void PrintSDLVersion()
{
	LogSDLVersion("Compiled with SDL", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
	int version = SDL_GetVersion();
	LogSDLVersion("Linked with SDL ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
	// LogSDLVersion("Compiled with SDL_image ",SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_MICRO_VERSION);
	// version = IMG_Version();
	// LogSDLVersion("Linked with SDL_image ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
	LogSDLVersion("Compiled with SDL_ttf ",	SDL_TTF_MAJOR_VERSION, SDL_TTF_MINOR_VERSION,SDL_TTF_MICRO_VERSION);
	version = TTF_Version();
	LogSDLVersion("Linked with SDL_ttf ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version),	SDL_VERSIONNUM_MICRO(version));
}

dae::Minigin::Minigin(const std::filesystem::path& dataPath)
{
	PrintSDLVersion();
	
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		SDL_Log("Renderer error: %s", SDL_GetError());
		throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
	}

	g_window = SDL_CreateWindow(
		"Programming 4 assignment",
		1024,
		576,
		SDL_WINDOW_OPENGL
	);
	if (g_window == nullptr) 
	{
		throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());
	}

	Renderer::GetInstance().Init(g_window);
	ResourceManager::GetInstance().Init(dataPath);

#ifdef USE_STEAMWORKS
	if (!SteamAPI_Init())
	{
		throw std::runtime_error("Fatal Error - Steam must be running to play this game (SteamAPI_Init() failed).");
	}
#endif
	// initialize frame time baseline
	m_lastFrameTime = std::chrono::steady_clock::now();
}

dae::Minigin::~Minigin()
{
	InputManager::GetInstance().ClearBindings();
	SceneManager::GetInstance().DestroyAll();
	Renderer::GetInstance().Destroy();
	EventQueue::GetInstance().Clear();
	// Ensure resource manager frees textures/fonts and shuts down SDL_ttf
	ResourceManager::GetInstance().Destroy();

#ifdef USE_STEAMWORKS
	SteamAPI_Shutdown();
#endif

	SDL_DestroyWindow(g_window);
	g_window = nullptr;
	SDL_Quit();
}

void dae::Minigin::Run(const std::function<void()>& load)
{
	load();
#ifndef __EMSCRIPTEN__
	// native loop - cap to ~60 FPS to avoid runaway CPU usage and to make timing stable
	constexpr std::chrono::duration<double, std::milli> targetFrameMs{ 16.6667 };
	while (!m_quit)
	{
		const auto frameStart = std::chrono::steady_clock::now();
		RunOneFrame();
		const auto frameTime = std::chrono::steady_clock::now() - frameStart;
		if (frameTime < targetFrameMs)
		{
			std::this_thread::sleep_for(targetFrameMs - frameTime);
		}
	}
#else
	// Emscripten provides browser-controlled main loop timing
	emscripten_set_main_loop_arg(&LoopCallback, this, 0, true);
#endif
}

void dae::Minigin::RunOneFrame()
{

	// Update the last-frame time (useful for components that may need delta later)
	const auto now = std::chrono::steady_clock::now();
	// store last-frame time for future use
	m_lastFrameTime = now;

	// Process input
	m_quit = !InputManager::GetInstance().ProcessInput();
	// Update scene (GameObject::Update now updates components)

#ifdef USE_STEAMWORKS
	SteamAPI_RunCallbacks();
#endif

	SceneManager::GetInstance().Update();
	EventQueue::GetInstance().ProcessEvents();
	// Render everything
	Renderer::GetInstance().Render();
}
