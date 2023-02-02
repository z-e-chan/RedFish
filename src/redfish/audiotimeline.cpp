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

#include "audiotimeline.h"

static constexpr int k_numMixItems = RF_MAX_VOICES * 2;

rf::AudioTimeline::AudioTimeline(int numChannels, int bufferSize, int sampleRate)
    : m_spec({bufferSize, sampleRate, numChannels})
    , m_voiceSet(&m_messenger, m_spec)
    , m_summingMixer(numChannels, bufferSize, sampleRate)
    , m_musicManager(this, m_spec)
{
    m_audioDataReferences = Allocator::AllocateArray<const AudioData*>("AudioDataReferences", RF_MAX_AUDIO_DATA);
    m_mixItems = Allocator::AllocateArray<MixItem>("MixItems", k_numMixItems, numChannels, bufferSize);
}

rf::AudioTimeline::~AudioTimeline()
{
    Allocator::DeallocateArray<const AudioData*>(&m_audioDataReferences, RF_MAX_AUDIO_DATA);
    Allocator::DeallocateArray<MixItem>(&m_mixItems, k_numMixItems);
}

void rf::AudioTimeline::Process(float* buffer, int size)
{
    memset(buffer, 0, size * sizeof(float));
    m_musicManager.Process(m_playhead, m_mixItems, &m_mixItemIndex);
    m_voiceSet.Process(m_playhead, m_mixItems, &m_mixItemIndex);
    m_summingMixer.Sum(buffer, m_mixItems, m_mixItemIndex, size, &m_messenger);
    m_mixItemIndex = 0;
    m_playhead += size;
    m_messenger.FlushMessages();
    HandleShutdown();
}

const long long& rf::AudioTimeline::GetPlayhead() const
{
    return m_playhead;
}

const rf::AudioSpec& rf::AudioTimeline::GetAudioSpec() const
{
    return m_spec;
}

const rf::AudioData* rf::AudioTimeline::GetAudioData(int index) const
{
    RF_ASSERT(index >= 0 && index < RF_MAX_AUDIO_DATA, "Index out of bounds");
    return m_audioDataReferences[index];
}

void rf::AudioTimeline::Shutdown()
{
    m_shutdownState = ShutdownState::Stop;
}

int rf::AudioTimeline::GetMaxNumMixItems()
{
    return k_numMixItems;
}

void rf::AudioTimeline::HandleShutdown()
{
    switch (m_shutdownState)
    {
        case ShutdownState::None: return;
        case ShutdownState::Stop:
        {
            m_musicManager.Stop(m_playhead, m_playhead);
            m_voiceSet.StopAll(m_playhead, m_playhead);
            m_shutdownState = ShutdownState::Stopping;
            break;
        }
        case ShutdownState::Stopping:
        {
            m_shutdownState = ShutdownState::SendShutdownCompleteMessage;
            break;
        }
        case ShutdownState::SendShutdownCompleteMessage:
        {
            Message msg;
            msg.m_type = MessageType::ContextShutdownComplete;
            m_messenger.AddMessage(msg);
            m_shutdownState = ShutdownState::Complete;
            break;
        }
        case ShutdownState::Complete: break;
        default: RF_FAIL("Invalid shutdown state.");
    }
}
