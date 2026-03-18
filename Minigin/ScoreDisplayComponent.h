#pragma once
#include "Component.h"
#include "IEventListener.h"

namespace dae
{
	class ScoreDisplayComponent final : public Component, public IEventListener
	{
	public:
		ScoreDisplayComponent(GameObject* owner, int playerId, int initialScore = 0);

		void Update() override {}
		void Render() const override {}

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(const Event& event) override;

	private:
		void RefreshText();

		int m_PlayerId{};
		int m_Score{};
	};
}
