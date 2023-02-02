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

#include "iir2highpassfilterdsp.h"

#include <cmath>

#include "buffer.h"
#include "mixitem.h"

rf::IIR2HighpassFilterDSP::IIR2HighpassFilterDSP(const AudioSpec& spec)
    : DSPBase(spec)
{
    ResetDelayLines();
}

void rf::IIR2HighpassFilterDSP::SetQ(float q)
{
    m_destinationQ = q;
}

void rf::IIR2HighpassFilterDSP::SetCutoff(float cutoff)
{
    m_destinationCutoff = cutoff;
}

void rf::IIR2HighpassFilterDSP::Process(MixItem* mixItem, int bufferSize)
{
    const bool noWorkToDo = m_startCutoff <= PluginUtils::k_minFilterCutoff && m_destinationCutoff <= PluginUtils::k_minFilterCutoff;
    if (m_bypass || noWorkToDo)
    {
        return;
    }

    const int numChannels = mixItem->m_channels;
    Buffer* mixItemBuffer = mixItem->m_arrayOfChannels;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        // Biquad Direct Form I
        for (int n = 0; n < bufferSize; ++n)
        {
            const float percent = n / static_cast<float>(bufferSize);
            const float lerpCutoff = ((1.0f - percent) * m_startCutoff) + percent * m_destinationCutoff;
            const float lerpQ = ((1.0f - percent) * m_startQ) + percent * m_destinationQ;

            const float w = (PluginUtils::k_twoPi * lerpCutoff) / m_spec.m_sampleRate;
            const float qInverse = 1.0f / lerpQ;
            const float B = ((1.0f - (qInverse * 0.5f) * std::sin(w)) / (1.0f + (qInverse * 0.5f) * std::sin(w))) * 0.5f;
            const float G = (0.5f + B) * std::cos(w);

            const float b0 = (0.5f + B + G) * 0.5f;
            const float b1 = -(0.5f + B + G);
            const float b2 = b0;
            const float a1 = -2.0f * G;
            const float a2 = 2.0f * B;

            const float inputSample = mixItemBuffer[channel][n];
            const float outputSample = b0 * inputSample + b1 * m_inputDelay[channel][0] + b2 * m_inputDelay[channel][1]
                                       - a1 * m_outputDelay[channel][0] - a2 * m_outputDelay[channel][1];

            mixItemBuffer[channel][n] = outputSample;

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
    }

    m_startCutoff = m_destinationCutoff;
    m_startQ = m_destinationQ;

    if (noWorkToDo)
    {
        ResetDelayLines();
    }
}

void rf::IIR2HighpassFilterDSP::ResetDelayLines()
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
