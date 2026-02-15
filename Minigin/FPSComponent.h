#pragma once
#include "Component.h"
#include <memory>
#include <SDL3/SDL.h>
#include <chrono>


namespace dae
{
	class Font;
	class Texture2D;

	class FPSComponent final : public Component
	{
	public:
		FPSComponent(GameObject* owner, std::shared_ptr<Font> font, const SDL_Color& color = { 255,255,255,255 });
		virtual ~FPSComponent() = default;

		void Update() override;
		void Render() const override;

	private:
		std::shared_ptr<Font> m_font{};
		SDL_Color m_color{};
		std::shared_ptr<Texture2D> m_textTexture{};
		double m_timer{ 0.0 };
		int m_frameCount{ 0 };
		bool m_needsUpdate{ true };

		// local steady clock state (used instead of global Time)
		std::chrono::steady_clock::time_point m_lastTime{ std::chrono::steady_clock::now() };
	};
}
