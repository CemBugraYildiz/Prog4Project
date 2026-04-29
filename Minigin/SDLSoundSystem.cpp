#include "SDLSoundSystem.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <string>
#include <algorithm>

namespace dae
{
    enum class SoundEventType { Play, PlayMusic, StopMusic, StopAll, Quit };

    struct SoundEvent
    {
        SoundEventType type{};
        SoundId        id{};
        float          volume{ 1.0f };
        bool           loop{ false };
    };

    class SDLSoundSystem::Impl
    {
    public:
        Impl()
        {
            if (!MIX_Init())
            {
                std::cerr << "MIX_Init failed: " << SDL_GetError() << "\n";
                return;
            }

            m_pMixer = MIX_CreateMixerDevice(
                SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                nullptr
            );

            if (!m_pMixer)
            {
                std::cerr << "MIX_CreateMixerDevice failed: "
                    << SDL_GetError() << "\n";
                MIX_Quit();
                return;
            }

            m_Thread = std::jthread(&Impl::WorkerThread, this);
            std::cout << "Sound System initialized on thread: "
                << m_Thread.get_id() << "\n";
        }

        ~Impl()
        {
            {
                std::lock_guard<std::mutex> queueLock(m_QueueMutex);
                m_EventQueue.push({ SoundEventType::Quit });
            }
            m_CondVar.notify_one();

            if (m_pMusicTrack)
            {
                MIX_DestroyTrack(m_pMusicTrack);
                m_pMusicTrack = nullptr;
            }

            for (auto& track : m_ActiveTracks)
            {
                MIX_DestroyTrack(track);
            }
            m_ActiveTracks.clear();

            for (auto& [id, audio] : m_AudioMap)
            {
                MIX_DestroyAudio(audio);
            }
            m_AudioMap.clear();

            if (m_pMixer)
            {
                MIX_DestroyMixer(m_pMixer);
                m_pMixer = nullptr;
            }

            MIX_Quit();
        }

        void AddEvent(SoundEvent event)
        {
            {
                std::lock_guard<std::mutex> queueLock(m_QueueMutex);
                m_EventQueue.push(std::move(event));
            }
            m_CondVar.notify_one();
        }

        void RegisterSound(SoundId id, const std::string& path)
        {
            std::lock_guard<std::mutex> audioLock(m_AudioMutex);

            if (m_AudioMap.contains(id)) return;

            if (!m_pMixer)
            {
                std::cerr << "Mixer not initialized!\n";
                return;
            }

            MIX_Audio* audio = MIX_LoadAudio(m_pMixer, path.c_str(), false);
            if (!audio)
            {
                std::cerr << "Failed to load: " << path
                    << " | " << SDL_GetError() << "\n";
                return;
            }

            m_AudioMap[id] = audio;
            std::cout << "Registered: " << path << "\n";
        }

    private:

        void WorkerThread()
        {
            std::cout << "Worker thread started: "
                << std::this_thread::get_id() << "\n";

            while (true)
            {
                SoundEvent event{};

                {
                    std::unique_lock<std::mutex> queueLock(m_QueueMutex);
                    m_CondVar.wait(queueLock, [this]
                        {
                            return !m_EventQueue.empty();
                        });

                    event = m_EventQueue.front();
                    m_EventQueue.pop();
                }
                switch (event.type)
                {
                case SoundEventType::Play:
                    HandlePlay(event.id, event.volume);
                    break;
                case SoundEventType::PlayMusic:
                    HandlePlayMusic(event.id, event.volume, event.loop);
                    break;
                case SoundEventType::StopMusic:
                    HandleStopMusic();
                    break;
                case SoundEventType::StopAll:
                    HandleStopAll();
                    break;
                case SoundEventType::Quit:
                    std::cout << "Worker thread stopping.\n";
                    return;
                }
            }
        }


        void HandlePlay(SoundId id, float volume)
        {
            std::lock_guard<std::mutex> audioLock(m_AudioMutex);

            if (!m_pMixer) return;

            auto it = m_AudioMap.find(id);
            if (it == m_AudioMap.end())
            {
                std::cerr << "Sound ID not found: " << id << "\n";
                return;
            }

            CleanupFinishedTracks();

            MIX_Track* track = MIX_CreateTrack(m_pMixer);
            if (!track)
            {
                std::cerr << "MIX_CreateTrack failed: "
                    << SDL_GetError() << "\n";
                return;
            }

            if (!MIX_SetTrackAudio(track, it->second))
            {
                std::cerr << "MIX_SetTrackAudio failed: "
                    << SDL_GetError() << "\n";
                MIX_DestroyTrack(track);
                return;
            }

            MIX_SetTrackGain(track, volume);

            if (!MIX_PlayTrack(track, 0))
            {
                std::cerr << "MIX_PlayTrack failed: "
                    << SDL_GetError() << "\n";
                MIX_DestroyTrack(track);
                return;
            }

            m_ActiveTracks.push_back(track);
        }

        void HandlePlayMusic(SoundId id, float volume, bool loop)
        {
            std::lock_guard<std::mutex> audioLock(m_AudioMutex);

            if (!m_pMixer) return;

            if (m_pMusicTrack)
            {
                MIX_StopTrack(m_pMusicTrack, 0);
                MIX_DestroyTrack(m_pMusicTrack);
                m_pMusicTrack = nullptr;
            }

            auto it = m_AudioMap.find(id);
            if (it == m_AudioMap.end())
            {
                std::cerr << "Music ID not found: " << id << "\n";
                return;
            }

            m_pMusicTrack = MIX_CreateTrack(m_pMixer);
            if (!m_pMusicTrack)
            {
                std::cerr << "MIX_CreateTrack (music) failed: "
                    << SDL_GetError() << "\n";
                return;
            }

            if (!MIX_SetTrackAudio(m_pMusicTrack, it->second))
            {
                MIX_DestroyTrack(m_pMusicTrack);
                m_pMusicTrack = nullptr;
                return;
            }

            MIX_SetTrackGain(m_pMusicTrack, volume);

            SDL_PropertiesID props = 0;
            if (loop)
            {
                props = SDL_CreateProperties();
                SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
            }

            if (!MIX_PlayTrack(m_pMusicTrack, props))
            {
                std::cerr << "MIX_PlayTrack (music) failed: "
                    << SDL_GetError() << "\n";
                MIX_DestroyTrack(m_pMusicTrack);
                m_pMusicTrack = nullptr;
            }

            if (props != 0)
            {
                SDL_DestroyProperties(props);
            }
        }

        void HandleStopMusic()
        {
            std::lock_guard<std::mutex> audioLock(m_AudioMutex);

            if (m_pMusicTrack)
            {
                MIX_StopTrack(m_pMusicTrack, 0);
                MIX_DestroyTrack(m_pMusicTrack);
                m_pMusicTrack = nullptr;
            }
        }

        void HandleStopAll()
        {
            std::lock_guard<std::mutex> audioLock(m_AudioMutex);

            if (m_pMusicTrack)
            {
                MIX_StopTrack(m_pMusicTrack, 0);
                MIX_DestroyTrack(m_pMusicTrack);
                m_pMusicTrack = nullptr;
            }

            if (m_pMixer)
            {
                MIX_StopAllTracks(m_pMixer, 0);
            }

            for (auto& track : m_ActiveTracks)
            {
                MIX_DestroyTrack(track);
            }
            m_ActiveTracks.clear();
        }

        void CleanupFinishedTracks()
        {
            m_ActiveTracks.erase(
                std::remove_if(
                    m_ActiveTracks.begin(),
                    m_ActiveTracks.end(),
                    [](MIX_Track* track)
                    {
                        bool finished = !MIX_TrackPlaying(track) &&
                            !MIX_TrackPaused(track);
                        if (finished)
                        {
                            MIX_DestroyTrack(track);
                        }
                        return finished;
                    }
                ),
                m_ActiveTracks.end()
            );
        }

        MIX_Mixer* m_pMixer{ nullptr };
        MIX_Track* m_pMusicTrack{ nullptr };

        std::unordered_map<SoundId, MIX_Audio*> m_AudioMap{};
        std::vector<MIX_Track*>                 m_ActiveTracks{};

        std::queue<SoundEvent>      m_EventQueue{};
        std::mutex                  m_QueueMutex{};
        std::condition_variable     m_CondVar{};

        std::mutex                  m_AudioMutex{};

        std::jthread                m_Thread{};
    };

    SDLSoundSystem::SDLSoundSystem()
        : m_pImpl{ std::make_unique<Impl>() } {
    }

    SDLSoundSystem::~SDLSoundSystem() = default;

    void SDLSoundSystem::Play(SoundId id, float volume)
    {
        m_pImpl->AddEvent({ SoundEventType::Play, id, volume, false });
    }

    void SDLSoundSystem::PlayMusic(SoundId id, float volume, bool loop)
    {
        m_pImpl->AddEvent({ SoundEventType::PlayMusic, id, volume, loop });
    }

    void SDLSoundSystem::StopMusic()
    {
        m_pImpl->AddEvent({ SoundEventType::StopMusic });
    }

    void SDLSoundSystem::StopAll()
    {
        m_pImpl->AddEvent({ SoundEventType::StopAll });
    }

    void SDLSoundSystem::RegisterSound(SoundId id, const std::string& filePath)
    {
        m_pImpl->RegisterSound(id, filePath);
    }
}