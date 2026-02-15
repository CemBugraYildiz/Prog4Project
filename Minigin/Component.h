#pragma once
#include <memory>

namespace dae
{
	class GameObject;

	class Component
	{
	protected:
		friend class GameObject;
		GameObject* m_owner{ nullptr };
		bool m_markedForRemoval{ false };
		bool m_active{ true };

		explicit Component(GameObject* owner) noexcept
			: m_owner(owner)
		{
		}

	public:
		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		Component& operator=(const Component& other) = delete;
		Component& operator=(Component&& other) = delete;

		virtual ~Component() = default;

		virtual void Update() {}
		virtual void Render() const {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		GameObject* GetOwner() const noexcept { return m_owner; }
		bool IsMarkedForRemoval() const noexcept { return m_markedForRemoval; }
		void MarkForRemoval() noexcept { m_markedForRemoval = true; }
		void SetActive(bool active) noexcept { m_active = active; }
		bool IsActive() const noexcept { return m_active; }
	};
}
