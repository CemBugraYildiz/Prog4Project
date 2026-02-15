#include "FPSComponent.h"
#include "Font.h"
#include "Texture2D.h"
#include "Renderer.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <sstream>
#include <SDL3/SDL.h>
#include <stdexcept>
#include "GameObject.h"
#include <chrono>

using namespace dae;

FPSComponent::FPSComponent(GameObject* owner, std::shared_ptr<Font> font, const SDL_Color& color)
	: Component(owner)
	, m_font(std::move(font))
	, m_color(color)
	, m_timer(0.0)
	, m_frameCount(0)
	, m_needsUpdate(true)
	, m_lastTime(std::chrono::steady_clock::now())
{
}

void FPSComponent::Update()
{
	// accumulate frames and measure elapsed time using steady_clock
	++m_frameCount;
	const auto now = std::chrono::steady_clock::now();
	const std::chrono::duration<double> dt = now - m_lastTime;
	m_lastTime = now;
	m_timer += dt.count();

	// update display every 0.5s
	if (m_timer >= 0.5)
	{
		const double fps = double(m_frameCount) / m_timer;
		m_frameCount = 0;
		m_timer = 0.0;

		std::stringstream ss;
		ss << static_cast<int>(fps + 0.5) << " FPS";

		// generate texture from text
		if (!m_font) return;

		const auto text = ss.str();
		const auto surf = TTF_RenderText_Blended(m_font->GetFont(), text.c_str(), (int)text.length(), m_color);
		if (surf == nullptr)
		{
			throw std::runtime_error(std::string("Render FPS text failed: ") + SDL_GetError());
		}
		SDL_Texture* sdlTex = SDL_CreateTextureFromSurface(Renderer::GetInstance().GetSDLRenderer(), surf);
		if (sdlTex == nullptr)
		{
			SDL_DestroySurface(surf);
			throw std::runtime_error(std::string("Create FPS text texture from surface failed: ") + SDL_GetError());
		}
		SDL_DestroySurface(surf);
		m_textTexture = std::make_shared<Texture2D>(sdlTex);
		m_needsUpdate = false;
	}
}

void FPSComponent::Render() const
{
	if (!m_textTexture || !m_owner) return;
	const auto& pos = m_owner->GetTransform().GetPosition();
	Renderer::GetInstance().RenderTexture(*m_textTexture, pos.x, pos.y);
}