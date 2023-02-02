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

#include "context.h"

#include "assetsystem.h"
#include "audiocallback.h"
#include "audiodata.h"
#include "audiotimeline.h"
#include "loadcommands.h"
#include "mixersystem.h"
#include "musicsystem.h"

rf::Context::Context(const Config& config)
    : m_spec {config.m_bufferSize, config.m_sampleRate, config.m_channels}
    , m_config(config)
{
    Allocator::SetCallbacks(config.m_onAllocate, config.m_onDeallocate);
    m_timeline = Allocator::Allocate<AudioTimeline>("AudioTimeline", m_config.m_channels, m_config.m_bufferSize, m_config.m_sampleRate);
    m_assetSystem = Allocator::Allocate<AssetSystem>("AssetSystem", &m_commandProcessor);
    m_mixerSystem = Allocator::Allocate<MixerSystem>("MixerSystem", this, &m_commandProcessor);
    m_musicSystem = Allocator::Allocate<MusicSystem>("MusicSystem", &m_commandProcessor, m_assetSystem);
    m_playingSoundInfo.reserve(RF_MAX_VOICES);
}

rf::Context::~Context()
{
    AudioCommand cmd;
    ShutdownCommand& data = EncodeAudioCommand<ShutdownCommand>(&cmd);
    m_commandProcessor.Add(cmd);

    bool waitForShutdown = true;
    while (waitForShutdown)
    {
        Message msg;
        while (m_timeline->m_messenger.Dequeue(msg))
        {
            if (msg.m_type == MessageType::ContextShutdownComplete)
            {
                waitForShutdown = false;
                break;
            }
        }
    }

    m_config.m_lockAudioDevice();
    m_audioCallback->Shutdown();
    Allocator::Deallocate<AudioTimeline>(&m_timeline);
    Allocator::Deallocate<AssetSystem>(&m_assetSystem);
    Allocator::Deallocate<MixerSystem>(&m_mixerSystem);
    Allocator::Deallocate<MusicSystem>(&m_musicSystem);
    m_timeline = nullptr;
    m_config.m_unlockAudioDevice();
}

void rf::Context::Update()
{
    Message msg;
    while (m_timeline->m_messenger.Dequeue(msg))
    {
        if (m_assetSystem->ProcessMessages(msg))
        {
            continue;
        }

        if (m_mixerSystem->ProcessMessages(msg))
        {
            continue;
        }

        if (m_musicSystem->ProcessMessages(msg))
        {
            continue;
        }

        switch (msg.m_type)
        {
            case MessageType::ContextVoiceStart:
            {
                const Message::ContextVoiceStartData& data = *msg.GetContextVoiceStartData();
                const AudioData* audioData = m_assetSystem->GetAudioData(data.m_audioHandle);
                m_playingSoundInfo.push_back({audioData->m_name, data.m_audioHandle});
                break;
            }
            case MessageType::ContextVoiceStop:
            {
                const Message::ContextVoiceStopData& data = *msg.GetContextVoiceStopData();
                const int size = static_cast<int>(m_playingSoundInfo.size());

                bool found = false;
                for (int i = 0; i < size; ++i)
                {
                    if (m_playingSoundInfo[i].m_audioHandle == data.m_audioHandle)
                    {
                        m_playingSoundInfo.erase(m_playingSoundInfo.begin() + i);
                        found = true;
                        break;
                    }
                }

                RF_ASSERT(found, "We are trying to remove a sound that we have not played.");
                break;
            }
            case MessageType::ContextNumVoices:
            {
                const Message::ContextNumVoicesData& data = *msg.GetContextNumVoicesData();
                m_numPlayingVoices = data.m_numVoices;
                break;
            }
            default: RF_FAIL("Message type not supported."); break;
        }
    }
}

rf::AssetSystem* rf::Context::GetAssetSystem()
{
    return m_assetSystem;
}

rf::MixerSystem* rf::Context::GetMixerSystem()
{
    return m_mixerSystem;
}

rf::MusicSystem* rf::Context::GetMusicSystem()
{
    return m_musicSystem;
}

const rf::AudioSpec& rf::Context::GetAudioSpec() const
{
    return m_spec;
}

int rf::Context::GetNumPlayingVoices() const
{
    return m_numPlayingVoices;
}

const std::vector<rf::PlayingSoundInfo>& rf::Context::GetPlayingSoundInfo() const
{
    return m_playingSoundInfo;
}

void rf::Context::OnAudioCallback(float* buffer, int size)
{
    if (m_timeline)
    {
        m_commandProcessor.Process(m_timeline);
        m_timeline->Process(buffer, size);
    }
}

void rf::Context::SetAudioCallback(AudioCallback* audioCallback)
{
    RF_ASSERT(!m_audioCallback, "Audio Callback has already been set.");
    m_audioCallback = audioCallback;
}