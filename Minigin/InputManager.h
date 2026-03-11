#pragma once
#include <memory>
#include <vector>
#include <SDL3/SDL.h>

#include "Singleton.h"
#include "Command.h"
#include "Controller.h"

namespace dae
{
	enum class InputState
	{
		Down,
		Up,
		Pressed
	};

	class InputManager final : public Singleton<InputManager>
	{
	public:
		InputManager();
		bool ProcessInput();

		void BindKeyboardCommand(SDL_Scancode key, InputState state, std::unique_ptr<Command> command);
		void BindControllerCommand(uint8_t controllerIndex, ControllerButton button, InputState state, std::unique_ptr<Command> command);

		void UnbindKeyboardCommand(SDL_Scancode key, InputState state);
		void UnbindControllerCommand(uint8_t controllerIndex, ControllerButton button, InputState state);

		void ClearBindings();

	private:
		struct KeyboardBinding
		{
			SDL_Scancode key{};
			InputState state{};
			std::unique_ptr<Command> command{};
		};

		struct ControllerBinding
		{
			uint8_t controllerIndex{};
			ControllerButton button{};
			InputState state{};
			std::unique_ptr<Command> command{};
		};

		bool IsTriggered(bool current, bool previous, InputState state) const;
		void HandleKeyboard();
		void HandleControllers();

		std::vector<KeyboardBinding> m_KeyboardBindings{};
		std::vector<ControllerBinding> m_ControllerBindings{};
		std::vector<uint8_t> m_PreviousKeyboardState{};
		std::vector<Controller> m_Controllers{};
	};
}
