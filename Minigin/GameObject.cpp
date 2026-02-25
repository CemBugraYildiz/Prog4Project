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

	// children are destroyed automatically (unique_ptr)
	m_children.clear();

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

	// Update children afterwards so they can read parent's final world transform
	for (auto& child : m_children)
	{
		if (child)
			child->Update();
	}

	// cleanup any components marked for removal
	CleanupRemovedComponents();

	// cleanup any children marked for removal
	CleanupRemovedChildren();

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

	// Render this object's texture at its world position
	const auto pos = GetWorldPosition();
	if (m_texture)
		Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);

	// Render children
	for (const auto& child : m_children)
	{
		if (child)
			child->Render();
	}
}

void GameObject::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void GameObject::SetPosition(float x, float y)
{
	// local position change -> mark dirty and propagate
	m_transform.SetPosition(x, y, 0.0f);
	MarkTransformDirtyRecursive();
}

glm::vec3 GameObject::GetWorldPosition() const noexcept
{
	if (m_isDirty)
	{
		// recompute
		const glm::vec3 local = m_transform.GetPosition();
		if (m_parent)
		{
			const glm::vec3 parentWorld = m_parent->GetWorldPosition();
			m_cachedWorldPosition = parentWorld + local;
		}
		else
		{
			m_cachedWorldPosition = local;
		}
		m_isDirty = false;
	}
	return m_cachedWorldPosition;
}

void GameObject::MarkTransformDirtyRecursive() noexcept
{
	m_isDirty = true;
	// children should also recompute world positions
	for (auto& child : m_children)
	{
		if (child)
			child->MarkTransformDirtyRecursive();
	}
}

GameObject* GameObject::AttachChild(std::unique_ptr<GameObject> child)
{
	if (!child) return nullptr;
	child->m_parent = this;
	// When attaching, child local position stays as-is; but cached world pos must be recomputed
	child->MarkTransformDirtyRecursive();
	auto ptr = child.get();
	m_children.emplace_back(std::move(child));
	return ptr;
}

std::unique_ptr<GameObject> GameObject::DetachChild(GameObject* child) noexcept
{
	if (child == nullptr) return nullptr;
	for (auto it = m_children.begin(); it != m_children.end(); ++it)
	{
		if (it->get() == child)
		{
			auto owned = std::move(*it);
			owned->m_parent = nullptr;
			owned->MarkTransformDirtyRecursive(); // world position now equals local until repositioned
			m_children.erase(it);
			return owned;
		}
	}
	return nullptr;
}

std::vector<GameObject*> GameObject::GetChildren() const noexcept
{
	std::vector<GameObject*> out;
	out.reserve(m_children.size());
	for (const auto& c : m_children)
		out.push_back(c.get());
	return out;
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

void GameObject::CleanupRemovedChildren() noexcept
{
	for (auto it = m_children.begin(); it != m_children.end(); )
	{
		if (*it && (*it)->IsMarkedForRemoval())
		{
			// detach first so components get OnDetach in destructor
			it = m_children.erase(it);
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