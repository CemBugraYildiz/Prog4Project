#pragma once
#include "Component.h"
#include <string>
#include <memory>

namespace dae
{
	class Texture2D;

	class RenderComponent final : public Component
	{
	public:
		// Construct with filename (loads texture) or with shared_ptr<Texture2D>
		RenderComponent(GameObject* owner, const std::string& filename);
		RenderComponent(GameObject* owner, std::shared_ptr<Texture2D> texture) noexcept;
		virtual ~RenderComponent() = default;

		void Update() override {}
		void Render() const override;

		void SetTexture(const std::string& filename);
		void SetTexture(std::shared_ptr<Texture2D> texture) noexcept;

	private:
		std::shared_ptr<Texture2D> m_texture{};
	};
}
