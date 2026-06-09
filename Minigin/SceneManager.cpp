#include "SceneManager.h"
#include "Scene.h"

void dae::SceneManager::Update()
{
	/*for(auto& scene : m_scenes)
	{
		scene->Update();
	}*/

	 if (m_ActiveScene)
	     m_ActiveScene->Update();
}

void dae::SceneManager::Render()
{
	for (const auto& scene : m_scenes)
	{
		scene->Render();
	}
}

void dae::SceneManager::DestroyAll()
{
	m_scenes.clear();
}

dae::Scene& dae::SceneManager::CreateScene()
{
	m_scenes.emplace_back(new Scene());
	if (m_scenes.size() == 1)
		m_ActiveScene = m_scenes.back().get();
	return *m_scenes.back();
}

void dae::SceneManager::SetActiveScene(int index)
{
	if (index >= 0 && index < static_cast<int>(m_scenes.size()))
	{
		m_ActiveScene = m_scenes[index].get();
	}
}
