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

#include "assetsystem.h"

#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION

#include <external/dr_libs/dr_flac.h>
#include <external/dr_libs/dr_wav.h>

#include "allocator.h"
#include "assert.h"
#include "commandprocessor.h"
#include "datacache.h"
#include "loadcommands.h"
#include "message.h"

rf::AssetSystem::AssetSystem(CommandProcessor* commands)
    : m_commands(commands)
{
    m_dataCache = Allocator::Allocate<DataCache>("DataCache");
}

rf::AssetSystem::~AssetSystem()
{
    Allocator::Deallocate<DataCache>(&m_dataCache);
}

rf::AudioHandle rf::AssetSystem::Load(float* interleavedSampleData, int numFrames, int channels, const char* name)
{
    const AudioHandle cachedHandle = m_dataCache->AssetExists(name);
    if (cachedHandle)
    {
        m_dataCache->IncrementReferenceCount(cachedHandle);
        return cachedHandle;
    }

    const int numSamples = numFrames * channels;
    const AudioHandle handle = m_dataCache->AllocateAudioData(interleavedSampleData, name, numSamples, channels);

    AudioCommand cmd;
    LoadAudioDataCommand& data = EncodeAudioCommand<LoadAudioDataCommand>(&cmd);
    data.m_index = m_dataCache->GetAudioDataIndex(handle);
    data.m_audioData = m_dataCache->GetAudioData(data.m_index);
    m_commands->Add(cmd);

    return handle;
}

rf::AudioHandle rf::AssetSystem::Load(const char* path)
{
    const AudioHandle cachedHandle = m_dataCache->AssetExists(path);
    if (cachedHandle)
    {
        m_dataCache->IncrementReferenceCount(cachedHandle);
        return cachedHandle;
    }

    const auto FindLastIndex = [](const char* str, const char find) {
        int index = -1;
        const size_t size = strlen(str);
        for (int i = 0; i < size; ++i)
        {
            if (str[i] == find)
            {
                index = i;
            }
        }
        return index;
    };

    char buffer[RF_MAX_NAME_SIZE] = {};
    const int startIndex = FindLastIndex(path, '.') + 1;
    const size_t size = strlen(path);
    int counter = 0;
    for (size_t i = startIndex; i < size; ++i)
    {
        buffer[counter++] = path[i];
    }

    if (strcmp(buffer, "flac") == 0)
    {
        return LoadFLACFile(path);
    }
    else if (strcmp(buffer, "wav") == 0)
    {
        return LoadWAVFile(path);
    }

    RF_FAIL("Unsupported file type. Only 'flac' and 'wav' is supported");
    return AudioHandle();
}

void rf::AssetSystem::Unload(const AudioHandle audioHandle)
{
    if (m_dataCache->DecrementReferenceCount(audioHandle))
    {
        AudioCommand cmd;
        UnloadAudioDataCommand& data = EncodeAudioCommand<UnloadAudioDataCommand>(&cmd);
        data.m_audioHandle = audioHandle;
        m_commands->Add(cmd);
    }
}

rf::AudioHandle rf::AssetSystem::LoadWAVFile(const char* path)
{
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    drwav_uint64 totalPCMFrameCount = 0;
    float* sampleData = drwav_open_file_and_read_pcm_frames_f32(path, &channels, &sampleRate, &totalPCMFrameCount, NULL);
    if (!sampleData)
    {
        RF_FAIL("Could not load WAV file");
        return AudioHandle();
    }

    const AudioHandle handle = Load(sampleData, static_cast<int>(totalPCMFrameCount), channels, path);
    drwav_free(sampleData, NULL);
    return handle;
}

rf::AudioHandle rf::AssetSystem::LoadFLACFile(const char* path)
{
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    drwav_uint64 totalPCMFrameCount = 0;
    float* sampleData = drflac_open_file_and_read_pcm_frames_f32(path, &channels, &sampleRate, &totalPCMFrameCount, NULL);
    if (!sampleData)
    {
        RF_FAIL("Could not load FLAC file");
        return AudioHandle();
    }

    const AudioHandle handle = Load(sampleData, static_cast<int>(totalPCMFrameCount), channels, path);
    drwav_free(sampleData, NULL);
    return handle;
}

const rf::AudioData* rf::AssetSystem::GetAudioData(AudioHandle audioHandle) const
{
    return m_dataCache->GetAudioData(audioHandle);
}

int rf::AssetSystem::GetAudioDataIndex(AudioHandle audioHandle) const
{
    return m_dataCache->GetAudioDataIndex(audioHandle);
}

bool rf::AssetSystem::ProcessMessages(const Message& message)
{
    switch (message.m_type)
    {
        case MessageType::AssetDelete:
        {
            const AudioHandle audioHandle = message.GetAssetDeleteData()->m_audioHandle;
            m_dataCache->DeallocateAudioData(audioHandle, m_commands);
            return true;
        }
        default: return false;
    }
}