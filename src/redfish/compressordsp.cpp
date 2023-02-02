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

#include "compressordsp.h"

#include <cmath>

#include "functions.h"
#include "mixitem.h"

rf::CompressorDSP::CompressorDSP(const AudioSpec& spec)
    : DSPBase(spec)
    , m_fader(spec.m_bufferSize)
{
}

void rf::CompressorDSP::SetThreshold(float threshold)
{
    m_thresholdDb = threshold;
}

void rf::CompressorDSP::SetRatio(float ratio)
{
    m_ratio = ratio;
}

void rf::CompressorDSP::SetMakeUpGainAmplitude(float makeUpGainAmplitude)
{
    m_makeUpGainAmplitude = makeUpGainAmplitude;
}

void rf::CompressorDSP::SetAttack(float attack)
{
    m_attack = attack;
}

void rf::CompressorDSP::SetRelease(float release)
{
    m_release = release;
}

void rf::CompressorDSP::Process(MixItem* mixItem, int bufferSize)
{
    if (m_bypass)
    {
        return;
    }

    const int numChannels = mixItem->m_channels;

    float maxAmp = -1.0f;
    for (int i = 0; i < numChannels; ++i)
    {
        for (int j = 0; j < bufferSize; ++j)
        {
            if (mixItem->m_arrayOfChannels[i][j] > maxAmp)
            {
                maxAmp = mixItem->m_arrayOfChannels[i][j];
            }
        }
    }
    const float mixItemDb = Functions::AmplitudeToDecibel(maxAmp);

    const int attackSamples = Functions::MsToSamples(m_attack, m_spec.m_sampleRate);
    const int releaseSamples = Functions::MsToSamples(m_release, m_spec.m_sampleRate);

    if (mixItemDb > m_thresholdDb)
    {
        m_state = State::Attack;

        // Find the difference between the compressed level and the input level and use it to
        // determine how much to turn down the input. We only support a 'hard knee'.
        const float diff = mixItemDb - m_thresholdDb;
        const float ratioedSignal = diff / m_ratio;
        const float totalSignal = m_thresholdDb + ratioedSignal;
        const float reductionDb = mixItemDb - totalSignal;
        const float reductionAmp = Functions::DecibelToAmplitude(-reductionDb);
        m_fader.Update(reductionAmp, 0, attackSamples);
    }
    else
    {
        if (m_state == State::Attack)
        {
            m_state = State::Release;
            m_fader.Update(1.0f, 0, releaseSamples);
        }
        else if (m_state == State::Release)
        {
            m_state = State::Steady;
        }
    }

    m_fader.Process(mixItem, bufferSize);

    // Apply make-up gain to the mix item.
    Buffer* mixItemBuffer = mixItem->m_arrayOfChannels;

    for (int i = 0; i < numChannels; ++i)
    {
        for (int j = 0; j < bufferSize; ++j)
        {
            mixItemBuffer[i][j] = mixItemBuffer[i][j] * m_makeUpGainAmplitude;
        }
    }
}