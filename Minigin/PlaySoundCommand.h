#pragma once
#include "Command.h"
#include "ServiceLocator.h"
#include "SoundIds.h"

namespace dae
{
    class PlaySoundCommand final : public Command
    {
    public:
        PlaySoundCommand(SoundId id, float volume = 1.0f)
            : m_Id{ id }
            , m_Volume{ volume }
        {
        }

        void Execute() override
        {
            ServiceLocator::GetSoundSystem().Play(m_Id, m_Volume);
        }

    private:
        SoundId m_Id;
        float   m_Volume;
    };

    class PlayMusicCommand final : public Command
    {
    public:
        PlayMusicCommand(SoundId id, float volume = 0.5f, bool loop = true)
            : m_Id{ id }
            , m_Volume{ volume }
            , m_Loop{ loop }
        {
        }

        void Execute() override
        {
            ServiceLocator::GetSoundSystem().PlayMusic(m_Id, m_Volume, m_Loop);
        }

    private:
        SoundId m_Id;
        float   m_Volume;
        bool    m_Loop;
    };

    class StopMusicCommand final : public Command
    {
    public:
        void Execute() override
        {
            ServiceLocator::GetSoundSystem().StopMusic();
        }
    };

    class StopAllSoundsCommand final : public Command
    {
    public:
        void Execute() override
        {
            ServiceLocator::GetSoundSystem().StopAll();
        }
    };

    class ToggleMusicCommand final : public Command
    {
    public:
        ToggleMusicCommand(SoundId id, float volume = 0.5f, bool loop = true)
            : m_Id{ id }
            , m_Volume{ volume }
            , m_Loop{ loop }
        {
        }

        void Execute() override
        {
            if (m_IsPlaying)
            {
                ServiceLocator::GetSoundSystem().StopMusic();
                m_IsPlaying = false;
            }
            else
            {
                ServiceLocator::GetSoundSystem().PlayMusic(
                    m_Id, m_Volume, m_Loop
                );
                m_IsPlaying = true;
            }
        }

    private:
        SoundId m_Id;
        float   m_Volume;
        bool    m_Loop;
        bool    m_IsPlaying{ true };
    };

    class MultiCommand final : public Command
    {
    public:
        void AddCommand(std::unique_ptr<Command> command)
        {
            m_Commands.push_back(std::move(command));
        }

        void Execute() override
        {
            for (auto& cmd : m_Commands)
            {
                cmd->Execute();
            }
        }

    private:
        std::vector<std::unique_ptr<Command>> m_Commands;
    };
}
