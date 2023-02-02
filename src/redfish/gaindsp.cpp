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

#include "gaindsp.h"

#include "functions.h"
#include "mixitem.h"

rf::GainDSP::GainDSP(const AudioSpec& spec)
    : DSPBase(spec)
    , m_amplitudeBuffer(spec.m_bufferSize)
{
}

void rf::GainDSP::SetAmplitude(float amplitude, bool interpolate)
{
    m_destinationAmplitude = amplitude;

    if (!interpolate)
    {
        m_startAmplitude = m_destinationAmplitude;
    }
}

void rf::GainDSP::SetAmplitude(float amplitude)
{
    SetAmplitude(amplitude, true);
}

void rf::GainDSP::Process(MixItem* mixItem, int bufferSize)
{
    if (m_bypass)
    {
        return;
    }

    if (Functions::FloatEquality(m_startAmplitude, 1.0f) && Functions::FloatEquality(m_destinationAmplitude, 1.0f))
    {
        return;
    }

    static constexpr int k_interpolationFrames = 32;
    static constexpr float k_inverse = 1.0f / k_interpolationFrames;

    for (int i = 0; i < bufferSize; ++i)
    {
        m_amplitudeBuffer[i] = i >= k_interpolationFrames
                                   ? m_destinationAmplitude
                                   : (m_startAmplitude * (1.0f - (k_inverse * i))) + (m_destinationAmplitude * (k_inverse * i));
    }

    m_startAmplitude = m_destinationAmplitude;

    const int numChannels = mixItem->m_channels;
    for (int i = 0; i < numChannels; ++i)
    {
        mixItem->m_arrayOfChannels[i].Multiply(m_amplitudeBuffer);
    }
}
