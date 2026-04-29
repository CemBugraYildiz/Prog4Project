#pragma once
#include "SoundSystem.h"
#include <memory>

namespace dae
{
    class ServiceLocator final
    {
    public:
        static SoundSystem& GetSoundSystem()
        {
            return *m_pSoundSystem;
        }

        static void RegisterSoundSystem(std::unique_ptr<SoundSystem> system)
        {
            if (system == nullptr)
                m_pSoundSystem = std::make_unique<NullSoundSystem>();
            else
                m_pSoundSystem = std::move(system);
        }

    private:
        static inline std::unique_ptr<SoundSystem> m_pSoundSystem
        { std::make_unique<NullSoundSystem>() };
    };
}
