#include "PepperDisplayComponent.h"
#include "TextComponent.h"
#include "EventQueue.h"
#include "Event.h"
#include "GameObject.h"

namespace BurgerTime
{
    PepperDisplayComponent::PepperDisplayComponent(dae::GameObject* owner, int playerId)
        : Component(owner), m_PlayerId(playerId) {
    }

    void PepperDisplayComponent::OnAttach()
    {
        dae::EventQueue::GetInstance().AddListener(dae::EventType::PepperUsed, this);
        RefreshText();
    }

    void PepperDisplayComponent::OnDetach()
    {
        dae::EventQueue::GetInstance().RemoveListener(dae::EventType::PepperUsed, this);
    }

    void PepperDisplayComponent::OnEvent(const dae::Event& event)
    {
        if (event.type == dae::EventType::PepperUsed && event.playerId == m_PlayerId)
        {
            m_PepperCount = event.value;
            RefreshText();
        }
    }

    void PepperDisplayComponent::RefreshText()
    {
        auto* tc = GetOwner()->GetComponent<dae::TextComponent>();
        if (tc) tc->SetText("Pepper: " + std::to_string(m_PepperCount));
    }
}