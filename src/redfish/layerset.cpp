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

#include "layerset.h"

#include "allocator.h"
#include "assert.h"
#include "audiospec.h"
#include "audiotimeline.h"
#include "musicvoice.h"

rf::LayerSet::LayerSet(const AudioSpec& spec, Messenger* messanger)
    : m_messanger(messanger)
    , m_bufferSize(spec.m_bufferSize)
{
    m_voices = Allocator::AllocateArray<MusicVoice>("MusicVoices", RF_MAX_CUE_LAYERS, spec);
    Reset();
}

rf::LayerSet::~LayerSet()
{
    Allocator::DeallocateArray<MusicVoice>(&m_voices, RF_MAX_CUE_LAYERS);
}

void rf::LayerSet::Play(const MusicTransitionRequest& request,
                        const MusicDatabase::TransitionData& transitionData,
                        const MusicDatabase::CueData& cueData,
                        const AudioData** audioData)
{
    m_numLayers = cueData.m_numLayers;
    for (int i = 0; i < m_numLayers; ++i)
    {
        m_voices[i].Play(request.m_startTime, cueData, transitionData.m_playCount, i, audioData);
    }
}

void rf::LayerSet::Reset()
{
    for (int i = 0; i < m_numLayers; ++i)
    {
        m_voices[i].ResetBase(m_messanger);
    }

    m_numLayers = 0;
}

rf::BaseVoice::Info rf::LayerSet::Process(long long playhead,
                                          int startingIndex,
                                          int fillSize,
                                          bool forceVoicesToDone,
                                          MixItem* outMixItems,
                                          int* outNumMixItems)
{
#if RF_ENABLE_ASSERTS
    BaseVoice::Info infoCheck[RF_MAX_CUE_LAYERS];
#endif

    BaseVoice::Info info;

    if (fillSize == 0)
    {
        return info;
    }

    for (int i = 0; i < m_numLayers; ++i)
    {
        MixItem* item = &outMixItems[(*outNumMixItems)++];
        RF_ASSERT(*outNumMixItems < AudioTimeline::GetMaxNumMixItems(), "Too many mix items will be generated. Increase RF_MAX_VOICES");
        info = m_voices[i].FillMixItemBase(playhead, item, startingIndex, fillSize, m_bufferSize, m_messanger);
        RF_ASSERT(item->m_mixGroupHandle, "Mix item has no mix group. This is incorrect.");

        if (info.m_done || forceVoicesToDone)
        {
            info.m_stopped = true;
        }

#if RF_ENABLE_ASSERTS
        infoCheck[i] = info;
#endif
    }

    if (info.m_stopped)
    {
        Reset();
    }

#if RF_ENABLE_ASSERTS
    // Remove the audio handle info from the compare because we'd expect the handles to be different for different layers.
    BaseVoice::Info compare1 = infoCheck[0];
    compare1.m_audioHandle = AudioHandle();
    for (int i = 0; i < m_numLayers; ++i)
    {
        BaseVoice::Info compare2 = infoCheck[i];
        compare2.m_audioHandle = AudioHandle();
        RF_ASSERT(compare1 == compare2, "Expected the same info");
    }
#endif

    return info;
}

bool rf::LayerSet::IsPlaying() const
{
    bool isPlaying = false;
    for (int i = 0; i < m_numLayers; ++i)
    {
        isPlaying = m_voices[i].IsPlaying() || isPlaying;
    }
    return isPlaying;
}