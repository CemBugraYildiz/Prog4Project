#pragma once
#include "Component.h"

namespace dae
{
	class ScoreComponent final : public Component
	{
	public:
		ScoreComponent(GameObject* owner, int playerId);

		void Update() override {}
		void Render() const override {}

		void AddScore(int amount);

		int GetPlayerId() const noexcept { return m_PlayerId; }
		int GetScore() const noexcept { return m_Score; }

	private:
		int m_PlayerId{};
		int m_Score{};
	};
}
