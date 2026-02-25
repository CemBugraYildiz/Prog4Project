#include "OrbitComponent.h"
#include "GameObject.h"
#include <glm/gtc/constants.hpp>
#include <cmath>

using namespace dae;

OrbitComponent::OrbitComponent(GameObject* owner, float radius, float angularSpeedRadPerSec, bool useParentAsCenter, const glm::vec2& fixedCenter, float initialAngle) noexcept
	: Component(owner)
	, m_radius(radius)
	, m_angle(initialAngle)
	, m_angularSpeed(angularSpeedRadPerSec)
	, m_useParentCenter(useParentAsCenter)
	, m_fixedCenter(fixedCenter)
	, m_lastTime(std::chrono::steady_clock::now())
{
}

void OrbitComponent::Update()
{
	const auto now = std::chrono::steady_clock::now();
	const std::chrono::duration<double> dt = now - m_lastTime;
	m_lastTime = now;
	const double delta = dt.count();

	m_angle += m_angularSpeed * static_cast<float>(delta);

	// compute center
	glm::vec2 center;
	if (m_useParentCenter && m_owner->GetParent())
	{
		const auto parentWorld = m_owner->GetParent()->GetWorldPosition();
		center = { parentWorld.x, parentWorld.y };
	}
	else
	{
		center = m_fixedCenter;
	}

	const float x = center.x + std::cos(m_angle) * m_radius;
	const float y = center.y + std::sin(m_angle) * m_radius;

	// If owner has parent and we want to keep the child attached, SetPosition expects local coordinates.
	// So compute local from desired world if parent exists.
	if (m_owner->GetParent())
	{
		const auto parentWorld = m_owner->GetParent()->GetWorldPosition();
		const float localX = x - parentWorld.x;
		const float localY = y - parentWorld.y;
		m_owner->SetPosition(localX, localY);
	}
	else
	{
		m_owner->SetPosition(x, y);
	}
}