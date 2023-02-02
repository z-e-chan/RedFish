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

#pragma once
#include "audiodata.h"
#include "defines.h"
#include "identifiers.h"

namespace rf
{
class CommandProcessor;

class DataCache
{
public:
    DataCache();
    DataCache(const DataCache&) = delete;
    DataCache(DataCache&&) = delete;
    DataCache& operator=(const DataCache&) = delete;
    DataCache& operator=(DataCache&&) = delete;
    ~DataCache();

    AudioHandle AllocateAudioData(const float* samples, const char* path, int numSamples, int numChannels);
    void DeallocateAudioData(AudioHandle audioHandle, CommandProcessor* commands);
    int GetAudioDataIndex(AudioHandle audioHandle) const;
    const AudioData* GetAudioData(AudioHandle audioHandle) const;
    const AudioData* GetAudioData(int index) const;

private:
    AudioHandle m_audioDataHandleLookupList[RF_MAX_AUDIO_DATA] = {};
    AudioData m_audioData[RF_MAX_AUDIO_DATA] = {};
};
}  // namespace rf