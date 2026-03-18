#pragma once
#include "Event.h"

namespace dae
{
	class IEventListener
	{
	public:
		virtual ~IEventListener() = default;
		virtual void OnEvent(const Event& event) = 0;
	};
}
