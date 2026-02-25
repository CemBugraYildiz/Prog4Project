#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <chrono>

namespace dae
{
	class OrbitComponent final : public Component
	{
	public:
		// If useParentAsCenter is true and owner has parent, the orbit center is the parent's world position.
		// Otherwise, a fixed world-space center is used.
		OrbitComponent(GameObject* owner, float radius, float angularSpeedRadPerSec, bool useParentAsCenter = true, const glm::vec2& fixedCenter = { 0,0 }, float initialAngle = 0.0f) noexcept;
		virtual ~OrbitComponent() = default;

		void Update() override;

	private:
		float m_radius;
		float m_angle;
		float m_angularSpeed;
		bool m_useParentCenter;
		glm::vec2 m_fixedCenter;
		std::chrono::steady_clock::time_point m_lastTime;
	};
}
