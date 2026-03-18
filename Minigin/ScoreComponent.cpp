#include "ScoreComponent.h"
#include "EventQueue.h"

dae::ScoreComponent::ScoreComponent(GameObject* owner, int playerId)
	: Component(owner)
	, m_PlayerId(playerId)
	, m_Score(0)
{
}

void dae::ScoreComponent::AddScore(int amount)
{
	m_Score += amount;

	dae::EventQueue::GetInstance().QueueEvent(
		dae::Event{ dae::EventType::ScoreChanged, m_PlayerId, m_Score }
	);
}