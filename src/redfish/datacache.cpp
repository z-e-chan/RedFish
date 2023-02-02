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

#include "datacache.h"

#include "allocator.h"
#include "assert.h"
#include "commandprocessor.h"
#include "loadcommands.h"

rf::DataCache::DataCache()
{
    for (int i = 0; i < RF_MAX_AUDIO_DATA; ++i)
    {
        m_audioDataHandleLookupList[i] = AudioHandle();
    }
}

rf::DataCache::~DataCache()
{
    for (int i = 0; i < RF_MAX_AUDIO_DATA; ++i)
    {
        m_audioData[i].Free();
    }
}

rf::AudioHandle rf::DataCache::AllocateAudioData(const float* samples, const char* path, int numSamples, int numChannels)
{
    for (int i = 0; i < RF_MAX_AUDIO_DATA; ++i)
    {
        if (!m_audioDataHandleLookupList[i])
        {
            const AudioHandle handle = CreateAudioHandle();
            m_audioDataHandleLookupList[i] = handle;
            AudioData& data = m_audioData[i];
            data.Allocate(numChannels, numSamples / numChannels, samples);
            data.m_name = path;
            return handle;
        }
    }

    RF_FAIL("Could not allocate audio data. Try increasing RF_MAX_AUDIO_DATA");
    return CreateAudioHandle();
}

void rf::DataCache::DeallocateAudioData(AudioHandle audioHandle, CommandProcessor* commands)
{
    for (int i = 0; i < RF_MAX_AUDIO_DATA; ++i)
    {
        if (m_audioDataHandleLookupList[i] == audioHandle)
        {
            m_audioDataHandleLookupList[i] = AudioHandle();
            m_audioData[i].Free();

            AudioCommand cmd;
            ClearAudioDataReferenceCommand& data = EncodeAudioCommand<ClearAudioDataReferenceCommand>(&cmd);
            data.m_index = i;
            commands->Add(cmd);

            return;
        }
    }

    RF_FAIL("Could not deallocate audio data.");
}

int rf::DataCache::GetAudioDataIndex(AudioHandle audioHandle) const
{
    for (int i = 0; i < RF_MAX_AUDIO_DATA; ++i)
    {
        if (m_audioDataHandleLookupList[i] == audioHandle)
        {
            return i;
        }
    }

    RF_FAIL("Could not find audio data.");
    return -1;
}

const rf::AudioData* rf::DataCache::GetAudioData(AudioHandle audioHandle) const
{
    for (int i = 0; i < RF_MAX_AUDIO_DATA; ++i)
    {
        if (m_audioDataHandleLookupList[i] == audioHandle)
        {
            return &m_audioData[i];
        }
    }

    RF_FAIL("Could not find audio data.");
    return &m_audioData[0];
}

const rf::AudioData* rf::DataCache::GetAudioData(int index) const
{
    RF_ASSERT(index >= 0 && index < RF_MAX_AUDIO_DATA, "Index out of bounds");
    return &m_audioData[index];
}