#include "AnimationComponent.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "RenderComponent.h"
#include "Renderer.h"
#include "Texture2D.h"
#include <iostream>
#include "EngineTime.h"

namespace BurgerTime
{
    AnimationComponent::AnimationComponent(dae::GameObject* owner)
        : Component(owner)
    {
    }

    void AnimationComponent::OnAttach()
    {
        std::cout << "AnimationComponent attached\n";
    }

    void AnimationComponent::OnDetach()
    {
        Stop();
    }

    // ============================================
    // UPDATE
    // ============================================
    void AnimationComponent::Update()
    {
        if (!IsActive() || !m_IsPlaying || !m_CurrentClip) return;

        m_FrameTimer += dae::EngineTime::GetDeltaTime();

        if (m_FrameTimer >= m_CurrentClip->frameTime)
        {
            m_FrameTimer = 0.0f;
            m_CurrentFrame++;

            // Check if animation finished
            if (m_CurrentFrame >= m_CurrentClip->frameCount)
            {
                if (m_CurrentClip->loop)
                {
                    m_CurrentFrame = 0; // Loop back
                }
                else
                {
                    m_CurrentFrame = m_CurrentClip->frameCount - 1; // Stay on last frame
                    m_HasFinished = true;
                    m_IsPlaying = false;

                    std::cout << "Animation '" << m_CurrentAnimationName << "' finished\n";

                    // TODO: Fire event (Observer pattern)
                    // dae::Event animEndEvent;
                    // animEndEvent.type = EventType::AnimationFinished;
                    // EventQueue::GetInstance().QueueEvent(animEndEvent);
                }
            }
        }
    }

    // ============================================
    // RENDER
    // ============================================
    void AnimationComponent::Render() const
    {
        if (!IsActive() || !m_CurrentTexture || !m_CurrentClip) return;

        auto pos = GetOwner()->GetWorldPosition();

        float textureWidth, textureHeight;
        SDL_GetTextureSize(m_CurrentTexture->GetSDLTexture(), &textureWidth, &textureHeight);

        // Calculate source rect (which frame to show)
        int frameWidth = static_cast<int>(textureWidth) / m_CurrentClip->columns;
        int frameHeight = static_cast<int>(textureHeight) / m_CurrentClip->rows;

        int row = m_CurrentFrame / m_CurrentClip->columns;
        int col = m_CurrentFrame % m_CurrentClip->columns;

        SDL_FRect srcRect{
            col * frameWidth,
            row * frameHeight,
            frameWidth,
            frameHeight
        };

        // Get render size from RenderComponent (if exists)
        float renderWidth = (m_RenderWidth > 0.0f) ? m_RenderWidth : static_cast<float>(frameWidth);
        float renderHeight = (m_RenderHeight > 0.0f) ? m_RenderHeight : static_cast<float>(frameHeight);

        // Flip handling
        SDL_FlipMode flip = (m_FlipX || m_CurrentClip->flipX)
            ? SDL_FLIP_HORIZONTAL
            : SDL_FLIP_NONE;

        dae::Renderer::GetInstance().RenderTexture(
            *m_CurrentTexture,
            pos.x, pos.y,
            renderWidth,    
            renderHeight,   
            &srcRect, flip
        );
    }

    // ============================================
    // ANIMATION CONTROL
    // ============================================
    void AnimationComponent::AddAnimation(const AnimationClip& clip)
    {
        m_Animations[clip.name] = clip;
        std::cout << "Added animation: " << clip.name
            << " (" << clip.frameCount << " frames)\n";
    }

    void AnimationComponent::Play(const std::string& animationName, bool forceRestart)
    {
        // Check if animation exists
        auto it = m_Animations.find(animationName);
        if (it == m_Animations.end())
        {
            std::cerr << " Animation '" << animationName << "' not found!\n";
            return;
        }

        // Don't restart if already playing (unless forced)
        if (!forceRestart && m_IsPlaying && m_CurrentAnimationName == animationName)
        {
            return;
        }

        // Switch animation
        m_CurrentAnimationName = animationName;
        m_CurrentClip = &it->second;

        // Load texture if different
        LoadTexture(m_CurrentClip->texturePath);

        // Reset playback
        ResetAnimation();

        std::cout << "Playing animation: " << animationName << "\n";
    }

    void AnimationComponent::Stop()
    {
        m_IsPlaying = false;
        m_CurrentFrame = 0;
        m_FrameTimer = 0.0f;
        m_HasFinished = false;
    }

    void AnimationComponent::Pause()
    {
        m_IsPlaying = false;
    }

    void AnimationComponent::Resume()
    {
        m_IsPlaying = true;
    }

    // ============================================
    // PRIVATE HELPERS
    // ============================================
    void AnimationComponent::LoadTexture(const std::string& path)
    {
        try
        {
            m_CurrentTexture = dae::ResourceManager::GetInstance().LoadTexture(path);
        }
        catch (const std::exception& e)
        {
            std::cerr << " Failed to load texture '" << path << "': " << e.what() << "\n";
            m_CurrentTexture = nullptr;
        }
    }

    void AnimationComponent::ResetAnimation()
    {
        m_CurrentFrame = 0;
        m_FrameTimer = 0.0f;
        m_IsPlaying = true;
        m_HasFinished = false;
    }
}