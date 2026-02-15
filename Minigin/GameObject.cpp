#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Component.h"

using namespace dae;

GameObject::~GameObject()
{
	// Ensure components get detached before they are destroyed
	for (auto& comp : m_components)
	{
		if (comp)
			comp->OnDetach();
	}
	m_components.clear();

	// Explicitly release texture (shared_ptr will decrease refcount)
	m_texture.reset();
}

void GameObject::Update()
{
	// Update components first
	for (auto& comp : m_components)
	{
		if (comp && comp->IsActive() && !comp->IsMarkedForRemoval())
			comp->Update();
	}

	// Default per-object logic (override in derived types)
}

void GameObject::Render() const
{
	// Components render first
	for (const auto& comp : m_components)
	{
		if (comp && comp->IsActive() && !comp->IsMarkedForRemoval())
			comp->Render();
	}

	// Original texture render
	const auto& pos = m_transform.GetPosition();
	if (m_texture)
		Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}

void GameObject::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void GameObject::SetPosition(float x, float y)
{
	m_transform.SetPosition(x, y, 0.0f);
}

void GameObject::CleanupRemovedComponents() noexcept
{
	for (auto it = m_components.begin(); it != m_components.end(); )
	{
		if (*it && (*it)->IsMarkedForRemoval())
		{
			(*it)->OnDetach();
			it = m_components.erase(it);
		}
		else
			++it;
	}
}

void GameObject::RemoveComponent(Component* component) noexcept
{
	if (component == nullptr) return;
	for (auto& comp : m_components)
	{
		if (comp.get() == component)
		{
			comp->MarkForRemoval();
			break;
		}
	}
	CleanupRemovedComponents();
}

void GameObject::RemoveAllComponents() noexcept
{
	for (auto& comp : m_components)
	{
		if (comp)
			comp->MarkForRemoval();
	}
	CleanupRemovedComponents();
}