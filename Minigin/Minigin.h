#pragma once
#include <string>
#include <functional>
#include <filesystem>
#include <chrono>

namespace dae
{
	class Minigin final
	{
		bool m_quit{};
		// track time between frames (native only). useful for future delta-time needs.
		std::chrono::steady_clock::time_point m_lastFrameTime{};
	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);
		void RunOneFrame();

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	};
}