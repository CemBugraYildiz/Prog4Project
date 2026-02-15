#include "RenderComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "GameObject.h"

using namespace dae;

RenderComponent::RenderComponent(GameObject* owner, const std::string& filename)
	: Component(owner)
{
	SetTexture(filename);
}

RenderComponent::RenderComponent(GameObject* owner, std::shared_ptr<Texture2D> texture) noexcept
	: Component(owner)
	, m_texture(std::move(texture))
{
}

void RenderComponent::Render() const
{
	if (!m_texture || !m_owner) return;
	const auto& pos = m_owner->GetTransform().GetPosition();
	Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}

void RenderComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture) noexcept
{
	m_texture = std::move(texture);
}