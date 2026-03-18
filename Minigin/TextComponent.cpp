#include "TextComponent.h"
#include "Font.h"
#include "Texture2D.h"
#include "Renderer.h"
#include "GameObject.h"

#include <SDL3_ttf/SDL_ttf.h>
#include <stdexcept>

using namespace dae;

TextComponent::TextComponent(GameObject* owner, const std::string& text, std::shared_ptr<Font> font, const SDL_Color& color)
	: Component(owner)
	, m_text(text)
	, m_color(color)
	, m_font(std::move(font))
	, m_textTexture(nullptr)
	, m_needsUpdate(true)
{
}

void TextComponent::Update()
{
	if (!m_needsUpdate)
		return;

	RebuildTexture();
}

void TextComponent::RebuildTexture()
{
	if (!m_font)
		return;

	const auto surf = TTF_RenderText_Blended(m_font->GetFont(), m_text.c_str(), m_text.length(), m_color);
	if (surf == nullptr)
	{
		throw std::runtime_error(std::string("Render text failed: ") + SDL_GetError());
	}

	auto texture = SDL_CreateTextureFromSurface(Renderer::GetInstance().GetSDLRenderer(), surf);
	if (texture == nullptr)
	{
		SDL_DestroySurface(surf);
		throw std::runtime_error(std::string("Create text texture from surface failed: ") + SDL_GetError());
	}

	SDL_DestroySurface(surf);

	m_textTexture = std::make_shared<Texture2D>(texture);
	m_needsUpdate = false;
}

void TextComponent::Render() const
{
	if (!m_textTexture || !m_owner)
		return;

	const auto pos = m_owner->GetWorldPosition();
	Renderer::GetInstance().RenderTexture(*m_textTexture, pos.x, pos.y);
}

void TextComponent::SetText(const std::string& text)
{
	if (m_text == text)
		return;

	m_text = text;
	m_needsUpdate = true;
}

void TextComponent::SetColor(const SDL_Color& color)
{
	m_color = color;
	m_needsUpdate = true;
}