#pragma once
#include "SoundSystem.h"
#include <memory>

namespace dae
{
    class SDLSoundSystem final : public SoundSystem
    {
    public:
        SDLSoundSystem();
        ~SDLSoundSystem() override;

        SDLSoundSystem(const SDLSoundSystem&) = delete;
        SDLSoundSystem& operator=(const SDLSoundSystem&) = delete;
        SDLSoundSystem(SDLSoundSystem&&) = delete;
        SDLSoundSystem& operator=(SDLSoundSystem&&) = delete;

        void Play(SoundId id, float volume) override;
        void PlayMusic(SoundId id, float volume, bool loop = true) override;
        void StopMusic() override;
        void StopAll() override;
        void RegisterSound(SoundId id, const std::string& filePath) override;

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}
