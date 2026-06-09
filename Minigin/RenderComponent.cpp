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
    if (!m_texture || !m_owner)
        return;

    const auto pos = m_owner->GetWorldPosition();

    // ============================================
    // GET RENDER SIZE
    // ============================================
    float renderWidth, renderHeight;
    GetRenderSize(renderWidth, renderHeight);

    // Apply scale
    renderWidth *= m_ScaleX;
    renderHeight *= m_ScaleY;

    // ============================================
    // RENDER
    // ============================================
    Renderer::GetInstance().RenderTexture(
        *m_texture,
        pos.x,
        pos.y,
        renderWidth,
        renderHeight
    );
}

void RenderComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture) noexcept
{
	m_texture = std::move(texture);
}
void RenderComponent::SetScale(float scaleX, float scaleY) noexcept
{
    m_ScaleX = scaleX;
    m_ScaleY = scaleY;
}

void RenderComponent::SetCustomSize(float width, float height) noexcept
{
    m_CustomWidth = width;
    m_CustomHeight = height;
}

void RenderComponent::ClearCustomSize() noexcept
{
    m_CustomWidth = -1.0f;
    m_CustomHeight = -1.0f;
}

void RenderComponent::GetRenderSize(float& outW, float& outH) const noexcept
{
    // Use custom size if set
    if (m_CustomWidth > 0.0f && m_CustomHeight > 0.0f)
    {
        outW = m_CustomWidth;
        outH = m_CustomHeight;
        return;
    }

    // Otherwise use texture size
    if (m_texture)
    {
        auto size = m_texture->GetSize();
        outW = size.x;
        outH = size.y;
    }
    else
    {
        outW = 0.0f;
        outH = 0.0f;
    }
}