#pragma once
#include "Component.h"
#include "IEventListener.h"

namespace dae
{
	class WinnerAchievementComponent final : public Component, public IEventListener
	{
	public:
		explicit WinnerAchievementComponent(GameObject* owner)
			: Component(owner)
		{
		}

		void Update() override {}
		void Render() const override {}

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(const Event& event) override;

	private:
		bool m_Unlocked{ false };
	};
}
