#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Scene.h"
#include "Singleton.h"

namespace dae
{
	class Scene;
	class SceneManager final : public Singleton<SceneManager>
	{
	public:
		Scene& CreateScene();

		void Update();
		void Render();
		void DestroyAll();
		void SetActiveScene(int index);
		Scene* GetActiveScene() const { return m_ActiveScene; }
		Scene& GetScene(int index) const { return *m_scenes[index]; }

	private:
		friend class Singleton<SceneManager>;
		SceneManager() = default;
		std::vector<std::unique_ptr<Scene>> m_scenes{};
		Scene* m_ActiveScene{ nullptr };
	};
}
