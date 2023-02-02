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

#include "basevoice.h"

#include "assert.h"
#include "audiodata.h"
#include "functions.h"
#include "mixitem.h"

rf::BaseVoice::BaseVoice(int bufferSize)
    : m_gain(bufferSize)
{
}

void rf::BaseVoice::PlayBase(const PlayParams& params)
{
    m_startTime = params.m_startTime;
    m_arrayOfChannels = params.m_audioData->m_arrayOfChannels;
    m_channels = params.m_audioData->m_numChannels;
    m_numFrames = params.m_audioData->m_numFrames;
    m_audioHandle = params.m_audioHandle;
    m_soundEffectHandle = params.m_soundEffectHandle;
    m_mixGroupHandle = params.m_mixGroupHandle;
    m_stingerHandle = params.m_stingerHandle;
    m_localPlayCount = 0;
    m_playCount = params.m_playCount;
    m_pitch = params.m_pitch;
    m_gain.SetAmplitude(params.m_amplitude, false);
    m_seek = 0;
    m_isPlaying = false;
}

rf::BaseVoice::Info rf::BaseVoice::FillMixItemBase(long long playhead,
                                                   MixItem* mixItem,
                                                   int startingIndex,
                                                   int fillSize,
                                                   int bufferSize,
                                                   Messenger* messenger)
{
    Info info;
    info.m_audioHandle = m_audioHandle;

    mixItem->ZeroOut();
    mixItem->m_mixGroupHandle = m_mixGroupHandle;

    if (!m_isPlaying)
    {
        const bool inFirstWindow = Functions::InFirstWindow(playhead, m_startTime, bufferSize);
        if (inFirstWindow)
        {
            Functions::SendVoiceStartMessage(*this, messenger);
            m_isPlaying = true;
            info.m_started = true;
        }
        else
        {
            return info;
        }
    }

    bool isOutOfSamples = false;
    const int difference = m_numFrames - m_seek;
    int maxFill = fillSize;
    if (difference <= fillSize)
    {
        maxFill = difference;
        // If we enter this loop it means that we do not have enough samples to fill the buffer.
        // Therefore, we are out of samples and should reset our seek when done.
        isOutOfSamples = true;
    }

    const int phase1FillAmount = maxFill;
    float index = 0.0f;
    int lastPlacementFrame = 0;

    for (int i = 0; i < m_channels; ++i)
    {
        index = 0.0f;
        for (int j = 0; j < phase1FillAmount; ++j)
        {
            const int intIndex = static_cast<int>(index);
            const int lookupFrame = m_seek + intIndex;
            if (lookupFrame >= m_numFrames)
            {
                break;
            }

            lastPlacementFrame = j + startingIndex;
            RF_ASSERT(i < m_channels, "Channel out of bounds");
            RF_ASSERT(lastPlacementFrame < bufferSize, "Frame out of bounds");
            mixItem->m_arrayOfChannels[i][lastPlacementFrame] = m_arrayOfChannels[i][lookupFrame];
            info.m_lastFilledFrame = lastPlacementFrame;
            index += m_pitch;
        }
    }

    const int intIndex = static_cast<int>(index);
    m_seek = m_seek + intIndex;
    if (isOutOfSamples)
    {
        m_seek = 0;
    }

    if (isOutOfSamples)
    {
        ++m_localPlayCount;

        if (m_playCount == 0 || (m_playCount > 1 && m_localPlayCount < m_playCount))
        {
            info.m_looped = true;
            const int numSamplesFilled = lastPlacementFrame + 1;
            const int phase2FillAmount = fillSize - numSamplesFilled;

            for (int i = 0; i < m_channels; ++i)
            {
                index = 0.0f;
                for (int j = 0; j < phase2FillAmount; ++j)
                {
                    const int lookupFrame = static_cast<int>(index);
                    RF_ASSERT(i < m_channels, "Channel out of bounds");
                    RF_ASSERT(lastPlacementFrame < lastPlacementFrame + 1 + j, "Frame out of bounds");
                    mixItem->m_arrayOfChannels[i][lastPlacementFrame + 1 + j] = m_arrayOfChannels[i][lookupFrame];
                    info.m_lastFilledFrame = lastPlacementFrame + 1 + j;
                    index += m_pitch;
                }
            }
            m_seek = static_cast<int>(index);
        }
        else
        {
            info.m_done = true;
        }
    }

    m_gain.Process(mixItem, bufferSize);

    info.m_mixItemFullyFilled = info.m_lastFilledFrame == bufferSize - 1;
    return info;
}

void rf::BaseVoice::ResetBase(Messenger* messenger)
{
    if (m_isPlaying)
    {
        Functions::SendVoiceStopMessage(*this, messenger);
    }

    m_startTime = -1;
    m_gain.SetAmplitude(1.0f, false);
    m_audioHandle = AudioHandle();
    m_soundEffectHandle = SoundEffectHandle();
    m_mixGroupHandle = MixGroupHandle();
    m_stingerHandle = StingerHandle();
    m_arrayOfChannels = nullptr;
    m_pitch = 1.0f;
    m_channels = 0;
    m_seek = 0;
    m_numFrames = 0;
    m_localPlayCount = 0;
    m_playCount = 0;
    m_isPlaying = false;
}

bool rf::BaseVoice::IsPlaying() const
{
    return m_isPlaying;
}

rf::AudioHandle rf::BaseVoice::GetAudioHandle() const
{
    return m_audioHandle;
}

rf::SoundEffectHandle rf::BaseVoice::GetSoundEffectHandle() const
{
    return m_soundEffectHandle;
}

rf::MixGroupHandle rf::BaseVoice::GetMixGroupHandle() const
{
    return m_mixGroupHandle;
}

rf::StingerHandle rf::BaseVoice::GetStingerHandle() const
{
    return m_stingerHandle;
}

long long rf::BaseVoice::GetStartTime() const
{
    return m_startTime;
}

bool rf::BaseVoice::Info::operator==(const Info& other) const
{
    return m_audioHandle == other.m_audioHandle && m_lastFilledFrame == other.m_lastFilledFrame && m_mixItemFullyFilled == other.m_mixItemFullyFilled
           && m_done == other.m_done && m_looped == other.m_looped && m_started == other.m_started && m_stopped == other.m_stopped;
}