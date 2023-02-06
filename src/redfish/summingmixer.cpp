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

#include "summingmixer.h"

#include "allocator.h"
#include "assert.h"
#include "dspbase.h"
#include "functions.h"
#include "messenger.h"

rf::SummingMixer::SummingMixer(int numChannels, int bufferSize, int sampleRate)
{
    m_mixGroups = Allocator::AllocateArray<MixGroupInternal>("MixGroupInternal", RF_MAX_MIX_GROUPS, numChannels, bufferSize, sampleRate);
    m_sends = Allocator::AllocateArray<SendInternal>("SendInternal", RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS);
    m_dsp = Allocator::AllocateArray<DSPBase*>("DSPBae", RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS);
}

rf::SummingMixer::~SummingMixer()
{
    Allocator::DeallocateArray<MixGroupInternal>(&m_mixGroups, RF_MAX_MIX_GROUPS);
    Allocator::DeallocateArray<SendInternal>(&m_sends, RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS);

    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        Allocator::Deallocate<DSPBase>(&m_dsp[i]);
    }

    Allocator::DeallocateArray<DSPBase*>(&m_dsp, RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS);
}

void rf::SummingMixer::CreateMixGroup(const MixGroupState& state)
{
    MixGroupInternal& mixGroup = m_mixGroups[m_numMixGroups++];
    mixGroup.m_state = state;
    mixGroup.m_isValid = true;
    Sort();
}

void rf::SummingMixer::DestroyMixGroup(int mixGroupIndex)
{
    m_mixGroups[mixGroupIndex] = m_mixGroups[m_numMixGroups - 1];
    --m_numMixGroups;
    Sort();
}

void rf::SummingMixer::Sum(void* buffer, MixItem* mixItems, int numMixItems, int bufferSize, Messenger* messenger)
{
    // Iterate through mix groups.
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        // Clean the mix group.
        MixGroupInternal& mixGroup = m_mixGroups[i];
        if (mixGroup.m_isValid)
        {
            MixItem* mixItem = &mixGroup.m_mixItem;
            MixGroupState* state = &mixGroup.m_state;

            mixItem->ZeroOut();

            // Iterate through mix items and try to find one with a matching mix group.
            for (int j = 0; j < numMixItems; ++j)
            {
                // If the mix group matches, sum it into the mix group.
                RF_ASSERT(mixItems[j].m_mixGroupHandle, "Expected a mix group ID");
                if (state->m_mixGroupHandle == mixItems[j].m_mixGroupHandle)
                {
                    mixItem->Sum(mixItems[j]);
                }
            }
        }
    }

    // Process each mix group.
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        MixGroupInternal& mixGroup = m_mixGroups[i];
        if (!mixGroup.m_isValid)
        {
            continue;
        }

        MixItem* mixItem = &m_mixGroups[i].m_mixItem;

        // Process plug-ins and fader.
        m_mixGroups[i].Process(mixItem, bufferSize, m_dsp, messenger);

        Message msg;
        msg.m_type = MessageType::MixGroupPeakAmplitude;
        Message::MixGroupPeakAmplitudeData* data = msg.GetMixGroupPeakAmplitudeData();
        data->m_mixGroupIndex = i;
        data->m_amplitude = mixItem->GetPeakAmplitude();
        messenger->AddMessage(msg);

        // Route signal to sends.
        for (int j = 0; j < RF_MAX_MIX_GROUP_SENDS; ++j)
        {
            const int sendIndex = m_mixGroups[i].m_state.m_sendSlots[j];
            if (sendIndex == -1)
            {
                continue;
            }

            const SendInternal& send = m_sends[sendIndex];
            const MixGroupHandle sendId = send.m_sendToMixGroupHandle;

            if (MixGroupInternal* mixGroup = MixGroupLookUp(sendId))
            {
                mixGroup->m_mixItem.Sum(*mixItem, send.m_amplitude);
            }
        }

        // Route signal to output.
        if (MixGroupInternal* mixGroup = MixGroupLookUp(m_mixGroups[i].m_state.m_outputMixGroupHandle))
        {
            if (!m_mixGroups[i].m_state.m_isMaster)
            {
                mixGroup->m_mixItem.Sum(*mixItem);
            }
        }
    }

    MixGroupInternal* masterMixGroup = MasterMixGroupLookUp();

    float* floatBuffer = reinterpret_cast<float*>(buffer);
    masterMixGroup->m_mixItem.ToInterleavedBuffer(floatBuffer);
}

rf::SummingMixer::MixGroupInternal* rf::SummingMixer::MixGroupLookUp(MixGroupHandle mixGroupHandle, int* outIndex)
{
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (m_mixGroups[i].m_state.m_mixGroupHandle == mixGroupHandle)
        {
            if (outIndex)
            {
                *outIndex = i;
            }

            return &m_mixGroups[i];
        }
    }

    RF_FAIL("Could not find Mix Group.");
    return nullptr;
}

rf::SummingMixer::MixGroupInternal* rf::SummingMixer::MixGroupLookUp(int index)
{
    return &m_mixGroups[index];
}

rf::SummingMixer::MixGroupInternal* rf::SummingMixer::MasterMixGroupLookUp(int* outIndex)
{
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (m_mixGroups[i].m_state.m_isMaster)
        {
            if (outIndex)
            {
                *outIndex = i;
            }

            return &m_mixGroups[i];
        }
    }

    RF_FAIL("Could not find Mix Group.");
    return nullptr;
}

void rf::SummingMixer::Sort()
{
    std::sort(m_mixGroups, m_mixGroups + m_numMixGroups, [](const MixGroupInternal& a, const MixGroupInternal& b) {
        return a.m_state.m_priority > b.m_state.m_priority;
    });
}

rf::SummingMixer::MixGroupInternal::MixGroupInternal(int channels, int bufferSize, int sampleRate)
    : m_mixItem(channels, bufferSize)
    , m_volume(bufferSize)
    , m_fader(bufferSize)
    , m_sampleRate(sampleRate)
{
}

void rf::SummingMixer::MixGroupInternal::UpdateVolume(float amplitude, float seconds)
{
    const int duration = Functions::MsToSamples(seconds * 1000.0f, m_sampleRate);
    m_volume.Update(amplitude, 0, duration);
}

void rf::SummingMixer::MixGroupInternal::FadeVolume(float amplitude, long long playhead, long long startTime, int duration)
{
    m_fader.Update(amplitude, startTime - playhead, duration);
}

void rf::SummingMixer::MixGroupInternal::Process(MixItem* mixItem, int bufferSize, DSPBase** dsp, Messenger* messenger)
{
    m_volume.Process(mixItem, bufferSize);
    m_fader.Process(mixItem, bufferSize);

    const bool fadeComplete = m_fader.GetIsFadeComplete();
    if (fadeComplete)
    {
        Message msg;
        msg.m_type = MessageType::MixGroupFadeComplete;
        Message::MixGroupFadeCompleteData* data = msg.GetMixGroupFadeCompleteData();
        data->m_mixGroupHandle = m_state.m_mixGroupHandle;
        data->m_amplitude = m_fader.GetAmplitude();
        messenger->AddMessage(msg);
    }

    for (int i = 0; i < RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        const int pluginIndex = m_state.m_pluginSlots[i];
        if (pluginIndex == -1)
        {
            continue;
        }

        dsp[pluginIndex]->Process(mixItem, bufferSize);
    }
}
