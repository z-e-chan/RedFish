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
#include "buffer.h"

namespace rf
{
struct MixItem;

class Fader
{
public:
    Fader(int bufferSize);

    void Reset();
    void Update(float amplitude, long long startTimeSamples, int durationSamples);
    bool Process_Part1_DoesFadeStartThisBuffer(int bufferSize, int* outProcessIndex);
    bool Process_Part2_ApplyTheFade(MixItem* item, int bufferSize, int processIndex);
    bool Process(MixItem* item, int bufferSize);
    float GetAmplitude() const;
    bool GetIsFadeComplete() const;
    bool IsFading() const;

private:
    struct FadeData
    {
        float m_startAmplitude = 1.0f;
        float m_destinationAmplitude = 1.0f;
        float m_durationInverse = 1.0f;
        int m_durationSamples = 0;
        int m_fadeSampleCounter = 0;
    };

    enum class State
    {
        StandBy,
        Pending,
        UpdateAmplitude,
        SetBufferToCurrentAmplitude
    };

    FadeData m_processingFadeData;
    FadeData m_pendingFadeData;
    State m_state = State::StandBy;
    Buffer m_amplitudeBuffer;
    long long m_numSamplesUntilStartProcess = 0;
    float m_currentAmplitude = 1.0f;
    bool m_isFadeComplete = false;
};

}  // namespace rf