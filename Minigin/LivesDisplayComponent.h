#pragma once
#include "Component.h"
#include "IEventListener.h"

namespace dae
{
	class LivesDisplayComponent final : public Component, public IEventListener
	{
	public:
		LivesDisplayComponent(GameObject* owner, int playerId, int initialLives);

		void Update() override {}
		void Render() const override {}

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(const Event& event) override;

	private:
		void RefreshText();

		int m_PlayerId{};
		int m_Lives{};
	};
}