#pragma once
#include <string>

namespace dae
{
    using SoundId = unsigned short;

    class SoundSystem
    {
    public:
        virtual ~SoundSystem() = default;
        virtual void Play(SoundId id, float volume) = 0;
        virtual void PlayMusic(SoundId id, float volume, bool loop = true) = 0;
        virtual void StopMusic() = 0;
        virtual void StopAll() = 0;
        virtual void RegisterSound(SoundId id, const std::string& filePath) = 0;
    };

    class NullSoundSystem final : public SoundSystem
    {
    public:
        void Play(SoundId, float) override {}
        void PlayMusic(SoundId, float, bool) override {}
        void StopMusic() override {}
        void StopAll() override {}
        void RegisterSound(SoundId, const std::string&) override {}
    };
}
