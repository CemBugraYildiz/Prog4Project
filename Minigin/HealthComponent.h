#pragma once
#include "Component.h"

namespace dae
{
	class HealthComponent final : public Component
	{
	public:
		HealthComponent(GameObject* owner, int playerId, int maxHealth = 1, int lives = 3);

		void Update() override {}
		void Render() const override {}

		void TakeDamage(int amount);

		int GetPlayerId() const noexcept { return m_PlayerId; }
		int GetLives() const noexcept { return m_Lives; }
		int GetHealth() const noexcept { return m_CurrentHealth; }

	private:
		int m_PlayerId{};
		int m_MaxHealth{};
		int m_CurrentHealth{};
		int m_Lives{};
	};
}
