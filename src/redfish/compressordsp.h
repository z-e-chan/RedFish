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
#include "dspbase.h"
#include "fader.h"

namespace rf
{
class CompressorDSP : public DSPBase
{
public:
    CompressorDSP(const AudioSpec& spec);

    void SetThreshold(float threshold);
    void SetRatio(float ratio);
    void SetMakeUpGainAmplitude(float makeUpGainAmplitude);
    void SetAttack(float attack);
    void SetRelease(float release);
    void Process(MixItem* mixItem, int bufferSize) override final;

private:
    enum class State
    {
        Steady,
        Attack,
        Release
    };

    State m_state = State::Steady;
    Fader m_fader;
    float m_thresholdDb = -24.0f;
    float m_ratio = 1.0f;
    float m_makeUpGainAmplitude = 1.0f;
    float m_attack = 10.0f;
    float m_release = 300.0f;
};
}  // namespace rf