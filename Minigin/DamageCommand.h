#pragma once
#include "Command.h"
#include "GameObject.h"
#include "HealthComponent.h"

namespace dae
{
	class DamageCommand final : public Command
	{
	public:
		DamageCommand(GameObject* gameObject, int damage = 1)
			: m_pGameObject(gameObject)
			, m_Damage(damage)
		{
		}

		void Execute() override
		{
			if (!m_pGameObject) return;

			auto* health = m_pGameObject->GetComponent<HealthComponent>();
			if (health)
			{
				health->TakeDamage(m_Damage);
			}
		}

	private:
		GameObject* m_pGameObject{};
		int m_Damage{};
	};
}
