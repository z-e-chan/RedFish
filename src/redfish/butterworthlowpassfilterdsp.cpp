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

#include "butterworthlowpassfilterdsp.h"

#include <cmath>

#include "buffer.h"
#include "mixitem.h"

rf::ButterworthLowpassFilterDSP::ButterworthLowpassFilterDSP(const AudioSpec& spec)
    : DSPBase(spec)
{
    ResetDelayLines();
}

void rf::ButterworthLowpassFilterDSP::SetCutoff(float cutoff)
{
    m_destinationCutoff = cutoff;
}

void rf::ButterworthLowpassFilterDSP::SetOrder(int order)
{
    m_order = order;
}

void rf::ButterworthLowpassFilterDSP::Process(MixItem* mixItem, int bufferSize)
{
    const bool noWorkToDo = m_startCutoff >= PluginUtils::k_maxFilterCutoff && m_destinationCutoff >= PluginUtils::k_maxFilterCutoff;
    if (m_bypass || noWorkToDo)
    {
        return;
    }

    switch (m_order)
    {
        case 1: ProcessFirstOrder(mixItem, bufferSize); break;
        case 2: ProcessSecondOrder(mixItem, bufferSize); break;
        default: ProcessFirstOrder(mixItem, bufferSize); break;
    }

    if (noWorkToDo)
    {
        ResetDelayLines();
    }
}

void rf::ButterworthLowpassFilterDSP::ResetDelayLines()
{
    for (int channel = 0; channel < PluginUtils::k_maxChannels; ++channel)
    {
        for (int n = 0; n < PluginUtils::k_maxFilterDelayLines; ++n)
        {
            m_inputDelay[channel][n] = 0.0f;
            m_outputDelay[channel][n] = 0.0f;
        }
    }
}

void rf::ButterworthLowpassFilterDSP::ProcessFirstOrder(MixItem* mixItem, int bufferSize)
{
    const int numChannels = mixItem->m_channels;
    Buffer* mixItemBuffer = mixItem->m_arrayOfChannels;

    float inputSample = 0.0f;
    float outputSample = 0.0f;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        for (int n = 0; n < bufferSize; ++n)
        {
            const float percent = n / static_cast<float>(bufferSize);
            const float lerpCutoff = ((1.0f - percent) * m_startCutoff) + percent * m_destinationCutoff;
            const float wo = lerpCutoff * PluginUtils::k_twoPi;
            const float A = std::tan(wo / (2.0f * m_spec.m_sampleRate));

            const float c1 = (A / (1.0f + A));
            const float c2 = ((-1.0f + A) / (1.0f + A));

            inputSample = mixItemBuffer[channel][n];

            outputSample = c1 * inputSample + c1 * m_inputDelay[channel][0] - c2 * m_outputDelay[channel][0];

            mixItemBuffer[channel][n] = outputSample;
            UpdateDelayLine(channel, inputSample, outputSample);
        }
    }

    m_startCutoff = m_destinationCutoff;
}

void rf::ButterworthLowpassFilterDSP::ProcessSecondOrder(MixItem* mixItem, int bufferSize)
{
    const int numChannels = mixItem->m_channels;
    Buffer* mixItemBuffer = mixItem->m_arrayOfChannels;

    float inputSample = 0.0f;
    float outputSample = 0.0f;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        for (int n = 0; n < bufferSize; ++n)
        {
            const float percent = n / static_cast<float>(bufferSize);
            const float lerpCutoff = ((1.0f - percent) * m_startCutoff) + percent * m_destinationCutoff;
            const float wo = lerpCutoff * PluginUtils::k_twoPi;
            const float A = std::tan(wo / (2.0f * m_spec.m_sampleRate));

            const float c1 = (A * A) / (1.0f + PluginUtils::k_sqrtTwo * A + A * A);
            const float c2 = 2.0f * c1;
            const float c3 = (-2.0f + 2.0f * (A * A)) / (1.0f + PluginUtils::k_sqrtTwo * A + A * A);
            const float c4 = (1.0f - PluginUtils::k_sqrtTwo * A + A * A) / (1.0f + PluginUtils::k_sqrtTwo * A + A * A);

            inputSample = mixItemBuffer[channel][n];

            outputSample = c1 * inputSample + c2 * m_inputDelay[channel][0] + c1 * m_inputDelay[channel][1] - c3 * m_outputDelay[channel][0]
                           - c4 * m_outputDelay[channel][1];

            mixItemBuffer[channel][n] = outputSample;
            UpdateDelayLine(channel, inputSample, outputSample);
        }
    }

    m_startCutoff = m_destinationCutoff;
}

void rf::ButterworthLowpassFilterDSP::UpdateDelayLine(int channel, float inputSample, float outputSample)
{
    // Delay of 2 samples.
    // x[n-2]
    m_inputDelay[channel][1] = m_inputDelay[channel][0];
    // y[n-2]
    m_outputDelay[channel][1] = m_outputDelay[channel][0];

    // Delay of 1 sample.
    // x[n-1]
    m_inputDelay[channel][0] = inputSample;
    // y[n-1]
    m_outputDelay[channel][0] = outputSample;
}