#include "HealthComponent.h"
#include "EventQueue.h"

dae::HealthComponent::HealthComponent(GameObject* owner, int playerId, int maxHealth, int lives)
	: Component(owner)
	, m_PlayerId(playerId)
	, m_MaxHealth(maxHealth)
	, m_CurrentHealth(maxHealth)
	, m_Lives(lives)
{
}

void dae::HealthComponent::TakeDamage(int amount)
{
	if (m_Lives <= 0)
		return;

	m_CurrentHealth -= amount;

	if (m_CurrentHealth > 0)
		return;

	--m_Lives;

	dae::EventQueue::GetInstance().QueueEvent(
		dae::Event{ dae::EventType::PlayerDied, m_PlayerId, m_Lives }
	);

	if (m_Lives > 0)
	{
		m_CurrentHealth = m_MaxHealth;
	}
}