#pragma once
#include <memory>
#include <string>
#include <vector>
#include "GameObject.h"

namespace dae
{
	class Scene final
	{
	public:
		void Add(std::unique_ptr<GameObject> object);
		void Remove(const GameObject& object);
		void RemoveAll();

		void Update();
		void Render() const;
		void Clear() noexcept;

		const std::vector<std::unique_ptr<GameObject>>& GetObjects() const noexcept { return m_objects; }

		~Scene() = default;
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

	private:
		friend class SceneManager;
		explicit Scene() = default;

		std::vector < std::unique_ptr<GameObject>> m_objects{};

		void CleanupRemovedObjects() noexcept;
	};

}
