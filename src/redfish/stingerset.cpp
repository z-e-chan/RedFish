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

#include "stingerset.h"

#include "allocator.h"
#include "assert.h"
#include "audiotimeline.h"
#include "functions.h"
#include "mixitem.h"
#include "musictransitionrequest.h"
#include "musicvoice.h"

static constexpr int k_maxStingerVoices = RF_MAX_CUE_LAYERS * RF_MAX_STINGERS;

rf::StingerSet::StingerSet(const AudioSpec& spec, Messenger* messanger)
    : m_messanger(messanger)
{
    m_voices = Allocator::AllocateArray<MusicVoice>("StingerVoices", k_maxStingerVoices, spec);
}

rf::StingerSet::~StingerSet()
{
    Allocator::DeallocateArray<MusicVoice>(&m_voices, k_maxStingerVoices);
}

void rf::StingerSet::Play(const MusicTransitionRequest& request,
                          const MusicDatabase::StingerData& stingerData,
                          const MusicDatabase::CueData& cueData,
                          const AudioData** audioData)
{
    RF_ASSERT(m_numStingers < k_maxStingerVoices, "Out of voices");
    const float stingerAmp = Functions::DecibelToAmplitude(stingerData.m_gainDb);
    const int numLayers = cueData.m_numLayers;
    for (int i = 0; i < numLayers; ++i)
    {
        if (m_numStingers < k_maxStingerVoices)
        {
            m_voices[m_numStingers++].Play(request.m_stingerStartTime, cueData, 1, i, audioData, stingerAmp);
        }
    }
}

void rf::StingerSet::Reset()
{
    for (int i = 0; i < m_numStingers; ++i)
    {
        m_voices[i].ResetBase(m_messanger);
    }

    m_numStingers = 0;
}

void rf::StingerSet::Process(long long playhead, int bufferSize, MixItem* outMixItems, int* outNumMixItems)
{
    for (int i = 0; i < m_numStingers; ++i)
    {
        RF_ASSERT(i >= 0, "Expected positive i");
        RF_ASSERT(i < m_numStingers, "i out of bounds");

        int startingIndex = 0;
        int fillLength = bufferSize;
        const long long startTime = m_voices[i].GetStartTime();
        const bool inFirstWindow = Functions::InFirstWindow(playhead, startTime, bufferSize);
        if (inFirstWindow)
        {
            const long long difference = startTime - playhead;
            RF_ASSERT(difference <= bufferSize, "Expected this value to be <= bufferSize.");
            startingIndex = static_cast<int>(difference);
            fillLength = bufferSize - startingIndex;
        }

        const bool process = m_voices[i].IsPlaying() || inFirstWindow;
        if (!process)
        {
            continue;
        }

        MixItem* item = &outMixItems[(*outNumMixItems)++];
        RF_ASSERT(*outNumMixItems < AudioTimeline::GetMaxNumMixItems(), "Too many mix items will be generated. Increase RF_MAX_VOICES");
        const BaseVoice::Info info = m_voices[i].FillMixItemBase(playhead, item, startingIndex, fillLength, bufferSize, m_messanger);
        RF_ASSERT(item->m_mixGroupHandle, "Mix item has no mix group. This is incorrect.");

        if (info.m_done)
        {
            m_voices[i].ResetBase(m_messanger);
            m_voices[i--] = m_voices[--m_numStingers];
        }
    }
}

void rf::StingerSet::ResetIfPlayingAudioHandle(AudioHandle audioHandle)
{
    for (int i = 0; i < m_numStingers; ++i)
    {
        if (m_voices[i].GetAudioHandle() == audioHandle)
        {
            Reset();
            break;
        }
    }
}