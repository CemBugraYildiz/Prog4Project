#pragma once
#include "Command.h"
#include "GameObject.h"
#include "ScoreComponent.h"

namespace dae
{
	class AddScoreCommand final : public Command
	{
	public:
		AddScoreCommand(GameObject* gameObject, int amount)
			: m_pGameObject(gameObject)
			, m_Amount(amount)
		{
		}

		void Execute() override
		{
			if (!m_pGameObject) return;

			auto* score = m_pGameObject->GetComponent<ScoreComponent>();
			if (score)
			{
				score->AddScore(m_Amount);
			}
		}

	private:
		GameObject* m_pGameObject{};
		int m_Amount{};
	};
}
