#include <algorithm>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objects.emplace_back(std::move(object));
}

void Scene::Remove(const GameObject& object)
{
	for (auto& obj : m_objects)
	{
		if (obj.get() == &object)
		{
			obj->MarkForRemoval();
			break;
		}
	}
}

void Scene::RemoveAll()
{
	for (auto& obj : m_objects)
	{
		if (obj)
			obj->MarkForRemoval();
	}
}

void Scene::Update()
{
	for (auto& object : m_objects)
	{
		if (object && !object->IsMarkedForRemoval())
			object->Update();
	}

	// After updating all objects, safely remove those marked for deletion
	CleanupRemovedObjects();
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		if (object && !object->IsMarkedForRemoval())
			object->Render();
	}
}

void Scene::CleanupRemovedObjects() noexcept
{
	for (auto it = m_objects.begin(); it != m_objects.end(); )
	{
		if (*it && (*it)->IsMarkedForRemoval())
		{
			it = m_objects.erase(it);
		}
		else
			++it;
	}
}
