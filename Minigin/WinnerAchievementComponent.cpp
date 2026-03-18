#include "WinnerAchievementComponent.h"
#include "EventQueue.h"

#ifdef USE_STEAMWORKS
#pragma warning(push)
#pragma warning(disable:4996)
#include <steam_api.h>
#pragma warning(pop)
#endif

void dae::WinnerAchievementComponent::OnAttach()
{
	dae::EventQueue::GetInstance().AddListener(dae::EventType::ScoreChanged, this);
}

void dae::WinnerAchievementComponent::OnDetach()
{
	dae::EventQueue::GetInstance().RemoveListener(dae::EventType::ScoreChanged, this);
}

void dae::WinnerAchievementComponent::OnEvent(const Event& event)
{
	if (m_Unlocked)
		return;

	if (event.type != dae::EventType::ScoreChanged)
		return;

	if (event.value < 500)
		return;

#ifdef USE_STEAMWORKS
	if (SteamUserStats())
	{
		if (SteamUserStats()->SetAchievement("ACH_WIN_ONE_GAME"))
		{
			SteamUserStats()->StoreStats();
			m_Unlocked = true;
		}
	}
#endif
}