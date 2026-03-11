#include "InputManager.h"

#include <algorithm>
#include <backends/imgui_impl_sdl3.h>

dae::InputManager::InputManager()
	: m_PreviousKeyboardState(SDL_SCANCODE_COUNT, 0)
{
	for (uint8_t i = 0; i < 4; ++i)
	{
		m_Controllers.emplace_back(i);
	}
}

bool dae::InputManager::ProcessInput()
{
	SDL_Event e{};
	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL3_ProcessEvent(&e);

		if (e.type == SDL_EVENT_QUIT)
		{
			return false;
		}
	}

	HandleKeyboard();
	HandleControllers();

	return true;
}

void dae::InputManager::BindKeyboardCommand(SDL_Scancode key, InputState state, std::unique_ptr<Command> command)
{
	if (!command) return;

	UnbindKeyboardCommand(key, state);
	m_KeyboardBindings.push_back({ key, state, std::move(command) });
}

void dae::InputManager::BindControllerCommand(uint8_t controllerIndex, ControllerButton button, InputState state, std::unique_ptr<Command> command)
{
	if (!command) return;

	UnbindControllerCommand(controllerIndex, button, state);
	m_ControllerBindings.push_back({ controllerIndex, button, state, std::move(command) });
}

void dae::InputManager::UnbindKeyboardCommand(SDL_Scancode key, InputState state)
{
	m_KeyboardBindings.erase(
		std::remove_if(m_KeyboardBindings.begin(), m_KeyboardBindings.end(),
			[key, state](const KeyboardBinding& binding)
			{
				return binding.key == key && binding.state == state;
			}),
		m_KeyboardBindings.end());
}

void dae::InputManager::UnbindControllerCommand(uint8_t controllerIndex, ControllerButton button, InputState state)
{
	m_ControllerBindings.erase(
		std::remove_if(m_ControllerBindings.begin(), m_ControllerBindings.end(),
			[controllerIndex, button, state](const ControllerBinding& binding)
			{
				return binding.controllerIndex == controllerIndex
					&& binding.button == button
					&& binding.state == state;
			}),
		m_ControllerBindings.end());
}

void dae::InputManager::ClearBindings()
{
	m_KeyboardBindings.clear();
	m_ControllerBindings.clear();
}

bool dae::InputManager::IsTriggered(bool current, bool previous, InputState state) const
{
	switch (state)
	{
	case InputState::Down:
		return current && !previous;
	case InputState::Up:
		return !current && previous;
	case InputState::Pressed:
		return current;
	default:
		return false;
	}
}

void dae::InputManager::HandleKeyboard()
{
	int numKeys{};
	const bool* currentState = SDL_GetKeyboardState(&numKeys);
	if (!currentState) return;

	if (static_cast<int>(m_PreviousKeyboardState.size()) < numKeys)
	{
		m_PreviousKeyboardState.resize(numKeys, 0);
	}

	for (auto& binding : m_KeyboardBindings)
	{
		const int key = static_cast<int>(binding.key);
		if (key < 0 || key >= numKeys || !binding.command) continue;

		const bool current = currentState[key];
		const bool previous = m_PreviousKeyboardState[key] != 0;

		if (IsTriggered(current, previous, binding.state))
		{
			binding.command->Execute();
		}
	}

	for (int i = 0; i < numKeys; ++i)
	{
		m_PreviousKeyboardState[i] = currentState[i] ? 1 : 0;
	}
}

void dae::InputManager::HandleControllers()
{
	for (auto& controller : m_Controllers)
	{
		controller.Update();
	}

	for (auto& binding : m_ControllerBindings)
	{
		if (binding.controllerIndex >= m_Controllers.size() || !binding.command)
		{
			continue;
		}

		Controller& controller = m_Controllers[binding.controllerIndex];
		const bool current = controller.IsDown(binding.button);
		const bool previous = controller.WasDown(binding.button);

		if (IsTriggered(current, previous, binding.state))
		{
			binding.command->Execute();
		}
	}
}