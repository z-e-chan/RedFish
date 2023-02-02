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

#include "limiterdsp.h"

#include <cmath>

#include "functions.h"
#include "mixitem.h"

rf::LimiterDSP::LimiterDSP(const AudioSpec& spec)
    : DSPBase(spec)
{
}

void rf::LimiterDSP::SetThreshold(float threshold)
{
    m_threshold = threshold;
}

void rf::LimiterDSP::Process(MixItem* mixItem, int)
{
    if (m_bypass)
    {
        return;
    }

    const float max = fabsf(mixItem->GetPeakAmplitude());
    const float maxDb = Functions::AmplitudeToDecibel(max);
    float difference = m_threshold - maxDb;

    if (difference < 0.0f)
    {
        const float ampDifference = Functions::DecibelToAmplitude(difference);
        const int numChannels = mixItem->m_channels;
        for (int i = 0; i < numChannels; ++i)
        {
            mixItem->m_arrayOfChannels[i].ScalarMultiply(ampDifference);
        }
    }
}