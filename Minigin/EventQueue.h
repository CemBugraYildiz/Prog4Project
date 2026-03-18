#pragma once
#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "Singleton.h"
#include "Event.h"
#include "IEventListener.h"

namespace dae
{
	struct EventTypeHash
	{
		size_t operator()(EventType type) const noexcept
		{
			return static_cast<size_t>(type);
		}
	};

	class EventQueue final : public Singleton<EventQueue>
	{
	public:
		void QueueEvent(const Event& event);
		void ProcessEvents();
		void Clear();

		void AddListener(EventType type, IEventListener* listener);
		void RemoveListener(EventType type, IEventListener* listener);

	private:
		friend class Singleton<EventQueue>;
		EventQueue() = default;

		std::queue<Event> m_Events{};
		std::unordered_map<EventType, std::vector<IEventListener*>, EventTypeHash> m_Listeners{};
	};
}
