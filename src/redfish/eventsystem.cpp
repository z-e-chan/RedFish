// MIT License

// Copyright (c) 2023 Zach Chan

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
