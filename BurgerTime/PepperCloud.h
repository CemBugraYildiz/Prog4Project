#pragma once
#include <Component.h>
#include "AnimationComponent.h"
#include "EngineTime.h"
#include "GameConfig.h"

namespace BurgerTime
{
    class PepperCloudComponent final : public dae::Component
    {
    public:
        explicit PepperCloudComponent(dae::GameObject* owner)
            : Component(owner) {
        }

        void OnAttach() override
        {
            auto* anim = GetOwner()->AddComponent<AnimationComponent>();
            anim->SetRenderSize(64.0f, 32.0f);

            AnimationClip cloud;
            cloud.name = "Pepper";
            cloud.texturePath = "BurgerTime/PeterPepper/Pepper.png";
            cloud.frameCount = 4;
            cloud.rows = 1;
            cloud.columns = 4;
            cloud.frameTime = 0.1f;
            cloud.loop = false;
            cloud.flipX = false;
            anim->AddAnimation(cloud);
            anim->Play("Pepper");
        }

        void Update() override
        {
            m_Timer -= dae::EngineTime::GetDeltaTime();
            if (m_Timer <= 0.0f)
                GetOwner()->MarkForRemoval();
        }

        void Render() const override {}

    private:
        float m_Timer{ 0.5f };
    };
}
