#pragma once
#include "Command.h"
#include "GameObject.h"

namespace dae
{
	class MoveCommand final : public Command
	{
	public:
		MoveCommand(GameObject* pGameObject, float dx, float dy)
			: m_pGameObject(pGameObject)
			, m_Dx(dx)
			, m_Dy(dy)
		{
		}

		void Execute() override
		{
			if (!m_pGameObject) return;

			const auto pos = m_pGameObject->GetTransform().GetPosition();
			m_pGameObject->SetPosition(pos.x + m_Dx, pos.y + m_Dy);
		}

	private:
		GameObject* m_pGameObject{};
		float m_Dx{};
		float m_Dy{};
	};
}