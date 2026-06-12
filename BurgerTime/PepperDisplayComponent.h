#pragma once
#include "Component.h"
#include "IEventListener.h"

namespace BurgerTime
{
    class PepperDisplayComponent final : public dae::Component
        , public dae::IEventListener
    {
    public:
        PepperDisplayComponent(dae::GameObject* owner, int playerId);
        void Update()  override {}
        void Render()  const override {}
        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(const dae::Event& event) override;
    private:
        void RefreshText();
        int m_PlayerId;
        int m_PepperCount{ 5 };
    };
}
