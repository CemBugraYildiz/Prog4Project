#pragma once
#include "Component.h"
#include <string>
#include <memory>

namespace dae
{
	class Texture2D;
	class GameObject;

	class RenderComponent final : public Component
	{
	public:
		RenderComponent(GameObject* owner, const std::string& filename);
		RenderComponent(GameObject* owner, std::shared_ptr<Texture2D> texture) noexcept;
		~RenderComponent() override = default;

		void Update() override {}
		void Render() const override;

		void SetTexture(const std::string& filename);
		void SetTexture(std::shared_ptr<Texture2D> texture) noexcept;

		void SetScale(float scaleX, float scaleY) noexcept;
		void SetScale(float uniformScale) noexcept { SetScale(uniformScale, uniformScale); }
		void GetScale(float& outX, float& outY) const noexcept { outX = m_ScaleX; outY = m_ScaleY; }

		void SetCustomSize(float width, float height) noexcept;
		void ClearCustomSize() noexcept;
		bool HasCustomSize() const noexcept { return m_CustomWidth > 0.0f; }
		void GetRenderSize(float& outW, float& outH) const noexcept;


	private:
		std::shared_ptr<Texture2D> m_texture{};

		// Scale factors
		float m_ScaleX{ 1.0f };
		float m_ScaleY{ 1.0f };

		// Custom size (-1 = use texture size)
		float m_CustomWidth{ -1.0f };
		float m_CustomHeight{ -1.0f };
	};
}