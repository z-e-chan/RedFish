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
#include "fader.h"
#include "mixgroupstate.h"
#include "mixitem.h"

namespace rf
{
class DSPBase;
class Messenger;

class SummingMixer
{
public:
    SummingMixer(int numChannels, int bufferSize, int sampleRate);
    SummingMixer(const SummingMixer&) = delete;
    SummingMixer(SummingMixer&&) = delete;
    SummingMixer& operator=(const SummingMixer&) = delete;
    SummingMixer& operator=(SummingMixer&&) = delete;
    ~SummingMixer();

    struct MixGroupInternal
    {
        MixGroupState m_state;
        MixItem m_mixItem;
        Fader m_volume;
        Fader m_fader;
        int m_sampleRate;
        bool m_isValid = false;

        MixGroupInternal(int channels, int bufferSize, int sampleRate);
        void UpdateVolume(float amplitude, float seconds);
        void FadeVolume(float amplitude, long long playhead, long long startTime, int duration);
        void Process(MixItem* mixItem, int bufferSize, DSPBase** dsp, Messenger* messenger);
    };

    struct SendInternal
    {
        float m_amplitude = 1.0f;
        MixGroupHandle m_sendToMixGroupHandle;
    };

    void CreateMixGroup(const MixGroupState& state);
    void DestroyMixGroup(int mixGroupIndex);
    void DestroyAllMixGroups();
    void Sum(void* buffer, MixItem* mixItems, int numMixItems, int bufferSize, Messenger* messenger);
    MixGroupInternal* MixGroupLookUp(MixGroupHandle mixGroupHandle, int* outIndex = nullptr);
    MixGroupInternal* MixGroupLookUp(int index);
    MixGroupInternal* MasterMixGroupLookUp(int* outIndex = nullptr);

    void Sort();

    SendInternal* m_sends = nullptr;
    DSPBase** m_dsp = nullptr;

private:
    MixGroupInternal* m_mixGroups = nullptr;
    int m_numMixGroups = 0;
};
}  // namespace rf