#pragma once
#include "Component.h"
#include <string>
#include <memory>
#include <vector>
#include <type_traits>
#include <algorithm>
#include "Transform.h"


namespace dae
{
	class Texture2D;
	class GameObject 
	{
		Transform m_transform{};
		std::shared_ptr<Texture2D> m_texture{};
		std::vector<std::unique_ptr<Component>> m_components{};

		// Scenegraph
			GameObject * m_parent{ nullptr };
		std::vector<std::unique_ptr<GameObject>> m_children{};

		// Dirty flag and cached world position
		mutable bool m_isDirty{ true };
		mutable glm::vec3 m_cachedWorldPosition{ 0.0f, 0.0f, 0.0f };

		// Removal flag (deferred deletion)
		bool m_markedForRemoval{ false };

		void CleanupRemovedComponents() noexcept;
		void CleanupRemovedChildren() noexcept;
		void MarkTransformDirtyRecursive() noexcept;
	public:
		// Existing API
		virtual void Update();
		virtual void Render() const;

		void SetTexture(const std::string& filename);
		void SetPosition(float x, float y);

		// mark for deletion (deferred)
		void MarkForRemoval() noexcept { m_markedForRemoval = true; }
		bool IsMarkedForRemoval() const noexcept { return m_markedForRemoval; }

		// Transform accessor for components
		Transform& GetTransform() noexcept { return m_transform; }
		const Transform& GetTransform() const noexcept { return m_transform; }

		// World position accessor (recomputes lazily using dirty flag)
		glm::vec3 GetWorldPosition() const noexcept;
		GameObject* GetParent() const noexcept { return m_parent; }

		// Children API
		// Attach a child; returns raw pointer to child (ownership is kept by parent)
		GameObject* AttachChild(std::unique_ptr<GameObject> child);
		// Detach child - ownership is transferred to caller. Returns nullptr if not found.
		std::unique_ptr<GameObject> DetachChild(GameObject* child) noexcept;
		// Iterate over children (returns vector of raw pointers)
		std::vector<GameObject*> GetChildren() const noexcept;

		// Component API
		template<typename T, typename... Args>
		T* AddComponent(Args&&... args);

		template<typename T>
		T* GetComponent() noexcept;

		template<typename T>
		bool HasComponent() const noexcept;

		template<typename T>
		void RemoveComponent() noexcept;

		void RemoveComponent(class Component* component) noexcept;
		void RemoveAllComponents() noexcept;

		GameObject() = default;
		virtual ~GameObject();
		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;
	};
	// Template implementations must be available to translation units that include GameObject.h
	template<typename T, typename... Args>
	T* GameObject::AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		// construct with owner pointer as first argument if T expects it
		auto comp = std::make_unique<T>(this, std::forward<Args>(args)...);
		T* ptr = comp.get();
		m_components.emplace_back(std::move(comp));
		ptr->OnAttach();
		return ptr;
	}

	template<typename T>
	T* GameObject::GetComponent() noexcept
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		for (const auto& comp : m_components)
		{
			if (auto casted = dynamic_cast<T*>(comp.get()))
				return casted;
		}
		return nullptr;
	}

	template<typename T>
	bool GameObject::HasComponent() const noexcept
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		for (const auto& comp : m_components)
		{
			if (dynamic_cast<const T*>(comp.get()) != nullptr)
				return true;
		}
		return false;
	}

	template<typename T>
	void GameObject::RemoveComponent() noexcept
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		for (auto& comp : m_components)
		{
			if (dynamic_cast<T*>(comp.get()) != nullptr)
			{
				comp->MarkForRemoval();
				return;
			}
		}
	}
}
