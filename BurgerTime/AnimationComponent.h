#pragma once
#include "Component.h"
#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include <unordered_map>

namespace dae
{
    class Texture2D;
}

namespace BurgerTime
{
    struct AnimationClip
    {
        std::string name;           // "Idle", "Walk", "Climb", etc.
        std::string texturePath;    // "BurgerTime/PeterPepper/Peter_Walk.png"
        int frameCount;             // 3 (1x3 sprite sheet için)
        int rows;                   // 1
        int columns;                // 3
        float frameTime;            // 0.1f = 10 FPS, 0.05f = 20 FPS
        bool loop;                  // true = tekrarla, false = bir kez oynat
        bool flipX;                 // Sprite'ý ters çevir
    };

    // ANIMATION COMPONENT
    class AnimationComponent final : public dae::Component
    {
    public:
        explicit AnimationComponent(dae::GameObject* owner);
        ~AnimationComponent() override = default;

        void Update() override;
        void Render() const override;

        void OnAttach() override;
        void OnDetach() override;

        // ANIMATION CONTROL
        void AddAnimation(const AnimationClip& clip);
        void Play(const std::string& animationName, bool forceRestart = false);
        void Stop();
        void Pause();
        void Resume();

        // GETTERS
        std::string GetCurrentAnimation() const { return m_CurrentAnimationName; }
        bool IsPlaying() const { return m_IsPlaying; }
        bool HasFinished() const { return m_HasFinished; }
        int GetCurrentFrame() const { return m_CurrentFrame; }

        // SETTINGS
        void SetFlipX(bool flip) { m_FlipX = flip; }
        bool GetFlipX() const { return m_FlipX; }
        void SetRenderSize(float width, float height)
        {
            m_RenderWidth = width;
            m_RenderHeight = height;
        }

    private:
        // Animation state
        std::unordered_map<std::string, AnimationClip> m_Animations;
        std::string m_CurrentAnimationName;
        AnimationClip* m_CurrentClip{ nullptr };

        // Playback state
        int m_CurrentFrame{ 0 };
        float m_FrameTimer{ 0.0f };
        bool m_IsPlaying{ false };
        bool m_HasFinished{ false };
        bool m_FlipX{ false };
        float m_RenderWidth{ 0.0f };
        float m_RenderHeight{ 0.0f };

        // Cached texture
        std::shared_ptr<dae::Texture2D> m_CurrentTexture{ nullptr };

        // Helper methods
        void LoadTexture(const std::string& path);
        void ResetAnimation();
    };
}
