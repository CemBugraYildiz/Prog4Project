#include "EventQueue.h"

void dae::EventQueue::QueueEvent(const Event& event)
{
	m_Events.push(event);
}

void dae::EventQueue::ProcessEvents()
{
	while (!m_Events.empty())
	{
		const Event event = m_Events.front();
		m_Events.pop();

		const auto found = m_Listeners.find(event.type);
		if (found == m_Listeners.end())
			continue;

		for (auto* listener : found->second)
		{
			if (listener)
			{
				listener->OnEvent(event);
			}
		}
	}
}

void dae::EventQueue::Clear()
{
	while (!m_Events.empty())
	{
		m_Events.pop();
	}
	m_Listeners.clear();
}

void dae::EventQueue::AddListener(EventType type, IEventListener* listener)
{
	if (!listener) return;

	auto& listeners = m_Listeners[type];
	if (std::find(listeners.begin(), listeners.end(), listener) == listeners.end())
	{
		listeners.push_back(listener);
	}
}

void dae::EventQueue::RemoveListener(EventType type, IEventListener* listener)
{
	const auto found = m_Listeners.find(type);
	if (found == m_Listeners.end())
		return;

	auto& listeners = found->second;
	listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}