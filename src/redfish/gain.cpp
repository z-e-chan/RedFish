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

#include "gain.h"

#include "functions.h"
#include "mixitem.h"

static constexpr int k_interpolationSamples = 32;
static constexpr float k_interpolationInverse = 1.0f / k_interpolationSamples;

rf::Gain::Gain(int bufferSize)
    : m_amplitudeBuffer(bufferSize)
{
    m_amplitudeBuffer.Set(1.0f);
}

void rf::Gain::SetAmplitude(float amplitude, bool interpolate)
{
    m_destinationAmplitude = amplitude;
    m_interpolate = interpolate;
}

void rf::Gain::Process(MixItem* item, int bufferSize)
{
    const float start = m_amplitudeBuffer[bufferSize - 1];
    if (Functions::FloatEquality(start, 1.0f) && Functions::FloatEquality(m_destinationAmplitude, 1.0f))
    {
        return;
    }

    if (m_interpolate)
    {
        for (int i = 0; i < k_interpolationSamples; ++i)
        {
            const float percent = k_interpolationInverse * i;
            m_amplitudeBuffer[i] = (start * (1.0f - percent)) + (m_destinationAmplitude * percent);
        }

        for (int i = k_interpolationSamples; i < bufferSize; ++i)
        {
            m_amplitudeBuffer[i] = m_destinationAmplitude;
        }
    }
    else
    {
        for (int i = 0; i < bufferSize; ++i)
        {
            m_amplitudeBuffer[i] = m_destinationAmplitude;
        }
    }

    const int numChannels = item->m_channels;
    Buffer* sampleData = item->m_arrayOfChannels;
    for (int i = 0; i < numChannels; ++i)
    {
        sampleData[i].Multiply(m_amplitudeBuffer);
    }
}