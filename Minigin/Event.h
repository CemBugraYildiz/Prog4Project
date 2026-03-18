#pragma once

namespace dae
{
	enum class EventType
	{
		PlayerDied,
		ScoreChanged
	};

	struct Event
	{
		EventType type{};
		int playerId{};
		int value{}; // remaining lives or current score
	};
}
