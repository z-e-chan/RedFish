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

#include "pandsp.h"

#include <cmath>

#include "buffer.h"
#include "defines.h"
#include "functions.h"
#include "mixitem.h"
#include "pluginutils.h"

rf::PanDSP::PanDSP(const AudioSpec& spec)
    : DSPBase(spec)
{
}

void rf::PanDSP::SetAngle(float angle, bool interpolate)
{
    m_angle = angle;
    if (!interpolate)
    {
        m_initialRadians = RadianMap(m_angle);
    }
}

void rf::PanDSP::SetAngle(float angle)
{
    SetAngle(angle, true);
}

void rf::PanDSP::Process(MixItem* mixItem, int bufferSize)
{
    if (m_bypass)
    {
        return;
    }

    const float radians = RadianMap(m_angle);
    const float lerpInverse = 1.0f / static_cast<float>(bufferSize - 2);

    for (int i = 0; i < bufferSize; ++i)
    {
        const float lerp = i * lerpInverse;
        const float lerpRadians = (m_initialRadians * (1.0f - (lerp)) + (radians * (lerp)));
        float leftAmp = 0.0f;
        float rightAmp = 0.0f;

#if RF_PAN_LAW_MINUS_THREE == 1
        leftAmp = cosf(lerpRadians);
        rightAmp = sinf(lerpRadians);
#elif RF_PAN_LAW_MINUS_FOUR_DOT_FIVE == 1
        leftAmp = sqrtf(((PluginUtils::k_piOverTwo - lerpRadians) * PluginUtils::k_twoOverPi * cosf(lerpRadians)));
        rightAmp = sqrtf((lerpRadians * PluginUtils::k_twoOverPi * sinf(lerpRadians)));
#elif RF_PAN_LAW_MINUS_SIX == 1
        leftAmp = (PluginUtils::k_piOverTwo - lerpRadians) * PluginUtils::k_twoOverPi;
        rightAmp = lerpRadians * PluginUtils::k_twoOverPi;
#endif

        Buffer* mixItemBuffer = mixItem->m_arrayOfChannels;
        const float tempLeft = (mixItemBuffer[0][i] * leftAmp) + (mixItemBuffer[1][i] * leftAmp);
        const float tempRight = (mixItemBuffer[0][i] * rightAmp) + (mixItemBuffer[1][i] * rightAmp);
        mixItemBuffer[0][i] = tempLeft;
        mixItemBuffer[1][i] = tempRight;
    }

    m_initialRadians = radians;
}

float rf::PanDSP::RadianMap(float angle)
{
    return Functions::Map(angle, -1.0f, 1.0, 0.0f, PluginUtils::k_piOverTwo);
}
