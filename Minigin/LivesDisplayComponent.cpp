#include "LivesDisplayComponent.h"
#include "EventQueue.h"
#include "TextComponent.h"
#include "GameObject.h"
#include <string>

dae::LivesDisplayComponent::LivesDisplayComponent(GameObject* owner, int playerId, int initialLives)
	: Component(owner)
	, m_PlayerId(playerId)
	, m_Lives(initialLives)
{
}

void dae::LivesDisplayComponent::OnAttach()
{
	dae::EventQueue::GetInstance().AddListener(dae::EventType::PlayerDied, this);
	RefreshText();
}

void dae::LivesDisplayComponent::OnDetach()
{
	dae::EventQueue::GetInstance().RemoveListener(dae::EventType::PlayerDied, this);
}

void dae::LivesDisplayComponent::OnEvent(const Event& event)
{
	if (event.type != dae::EventType::PlayerDied)
		return;

	if (event.playerId != m_PlayerId)
		return;

	m_Lives = event.value;
	RefreshText();
}

void dae::LivesDisplayComponent::RefreshText()
{
	auto* text = m_owner->GetComponent<dae::TextComponent>();
	if (!text) return;

	text->SetText("# lives: " + std::to_string(m_Lives));
}