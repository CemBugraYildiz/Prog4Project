#pragma once
#include <cstdint>
#include <memory>

namespace dae
{
	enum class ControllerButton : uint16_t
	{
		DPadUp = 0x0001,
		DPadDown = 0x0002,
		DPadLeft = 0x0004,
		DPadRight = 0x0008,
		Start = 0x0010,
		Back = 0x0020,
		LeftThumb = 0x0040,
		RightThumb = 0x0080,
		LeftShoulder = 0x0100,
		RightShoulder = 0x0200,
		A = 0x1000,
		B = 0x2000,
		X = 0x4000,
		Y = 0x8000
	};

	class Controller final
	{
	public:
		explicit Controller(uint8_t index);
		~Controller();

		Controller(const Controller&) = delete;
		Controller& operator=(const Controller&) = delete;
		Controller(Controller&&) noexcept;
		Controller& operator=(Controller&&) noexcept;

		void Update();
		bool IsConnected() const;
		bool IsDown(ControllerButton button) const;
		bool WasDown(ControllerButton button) const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}
