#include "Controller.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#endif

class dae::Controller::Impl
{
public:
#ifdef _WIN32
	explicit Impl(uint8_t index)
		: m_Index(index)
	{
	}

	void Update()
	{
		m_PreviousButtons = m_CurrentButtons;

		XINPUT_STATE state{};
		const DWORD result = XInputGetState(m_Index, &state);

		if (result == ERROR_SUCCESS)
		{
			m_Connected = true;
			m_CurrentButtons = state.Gamepad.wButtons;
		}
		else
		{
			m_Connected = false;
			m_CurrentButtons = 0;
		}
	}

	bool IsConnected() const
	{
		return m_Connected;
	}

	bool IsDown(dae::ControllerButton button) const
	{
		return (m_CurrentButtons & static_cast<WORD>(button)) != 0;
	}

	bool WasDown(dae::ControllerButton button) const
	{
		return (m_PreviousButtons & static_cast<WORD>(button)) != 0;
	}

private:
	uint8_t m_Index{};
	WORD m_CurrentButtons{};
	WORD m_PreviousButtons{};
	bool m_Connected{};
#else
	explicit Impl(uint8_t) {}
	void Update() {}
	bool IsConnected() const { return false; }
	bool IsDown(dae::ControllerButton) const { return false; }
	bool WasDown(dae::ControllerButton) const { return false; }
#endif
};

dae::Controller::Controller(uint8_t index)
	: m_pImpl(std::make_unique<Impl>(index))
{
}

dae::Controller::~Controller() = default;
dae::Controller::Controller(Controller&&) noexcept = default;
dae::Controller& dae::Controller::operator=(Controller&&) noexcept = default;

void dae::Controller::Update()
{
	m_pImpl->Update();
}

bool dae::Controller::IsConnected() const
{
	return m_pImpl->IsConnected();
}

bool dae::Controller::IsDown(ControllerButton button) const
{
	return m_pImpl->IsDown(button);
}

bool dae::Controller::WasDown(ControllerButton button) const
{
	return m_pImpl->WasDown(button);
}