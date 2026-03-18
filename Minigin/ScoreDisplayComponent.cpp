#include "ScoreDisplayComponent.h"
#include "EventQueue.h"
#include "TextComponent.h"
#include "GameObject.h"
#include <string>

dae::ScoreDisplayComponent::ScoreDisplayComponent(GameObject* owner, int playerId, int initialScore)
	: Component(owner)
	, m_PlayerId(playerId)
	, m_Score(initialScore)
{
}

void dae::ScoreDisplayComponent::OnAttach()
{
	dae::EventQueue::GetInstance().AddListener(dae::EventType::ScoreChanged, this);
	RefreshText();
}

void dae::ScoreDisplayComponent::OnDetach()
{
	dae::EventQueue::GetInstance().RemoveListener(dae::EventType::ScoreChanged, this);
}

void dae::ScoreDisplayComponent::OnEvent(const Event& event)
{
	if (event.type != dae::EventType::ScoreChanged)
		return;

	if (event.playerId != m_PlayerId)
		return;

	m_Score = event.value;
	RefreshText();
}

void dae::ScoreDisplayComponent::RefreshText()
{
	auto* text = m_owner->GetComponent<dae::TextComponent>();
	if (!text) return;

	text->SetText("Score: " + std::to_string(m_Score));
}