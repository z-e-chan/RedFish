#include "eventsystem.h"

#include "message.h"

rf::EventSystem::EventSystem(CommandProcessor* commands)
    : m_commands(commands)
{
}

void rf::EventSystem::SetUserData(void* userData)
{
    m_userData = userData;
}

void rf::EventSystem::RegisterOnBar(void (*onBar)(int bar, int beat, void* userData))
{
    m_onBar = onBar;
}

void rf::EventSystem::RegisterOnBeat(void (*onBeat)(int bar, int beat, void* userData))
{
    m_onBeat = onBeat;
}

void rf::EventSystem::RegisterOnMusicFinished(void (*onMusicFinished)(void* userData))
{
    m_onMusicFinished = onMusicFinished;
}

void rf::EventSystem::ProcessMessages(const Message& message)
{
    switch (message.m_type)
    {
        case MessageType::MusicBarChanged:
        {
            if (m_onBar)
            {
                const Message::MusicBarChangedData* data = message.GetMusicBarChangedData();
                m_onBar(data->m_currentBar, data->m_currentBeat, m_userData);
            }
            break;
        }
        case MessageType::MusicBeatChanged:
        {
            if (m_onBeat)
            {
                const Message::MusicBeatChangedData* data = message.GetMusicBeatChangedData();
                m_onBeat(data->m_currentBar, data->m_currentBeat, m_userData);
            }
            break;
        }
        case MessageType::MusicFinished:
        {
            if (m_onMusicFinished)
            {
                m_onMusicFinished(m_userData);
            }
            break;
        }
        default: break;
    }
}
