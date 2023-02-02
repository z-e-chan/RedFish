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

#include "fader.h"

#include <algorithm>

#include "assert.h"
#include "mixitem.h"

rf::Fader::Fader(int bufferSize)
    : m_amplitudeBuffer(bufferSize)
{
    m_amplitudeBuffer.Set(1.0f);
}

void rf::Fader::Reset()
{
    m_state = State::StandBy;
    m_currentAmplitude = 1.0f;
    m_numSamplesUntilStartProcess = 0;
    m_processingFadeData = FadeData();
    m_pendingFadeData = FadeData();
    m_amplitudeBuffer.Set(m_currentAmplitude);
}

void rf::Fader::Update(float amplitude, long long startTimeSamples, int durationSamples)
{
    if (durationSamples <= 0)
    {
        durationSamples = 1;
    }

    m_state = State::Pending;

    m_pendingFadeData.m_destinationAmplitude = amplitude;
    m_pendingFadeData.m_durationSamples = durationSamples;
    m_pendingFadeData.m_durationInverse = 1.0f / durationSamples;
    m_numSamplesUntilStartProcess = startTimeSamples;
}

bool rf::Fader::Process_Part1_DoesFadeStartThisBuffer(int bufferSize, int* outProcessIndex)
{
    m_isFadeComplete = false;
    int processIndex = 0;
    *outProcessIndex = processIndex;

    bool returnValue = false;

    if (m_state == State::SetBufferToCurrentAmplitude)
    {
        for (int i = 0; i < bufferSize; ++i)
        {
            m_amplitudeBuffer[i] = m_currentAmplitude;
        }
        m_isFadeComplete = true;
        m_state = State::StandBy;
    }
    else if (m_state == State::Pending)
    {
        // Start the fade in this buffer
        if (m_numSamplesUntilStartProcess < bufferSize)
        {
            m_state = State::UpdateAmplitude;
            m_processingFadeData = m_pendingFadeData;
            m_processingFadeData.m_startAmplitude = m_currentAmplitude;

            RF_ASSERT(
                m_numSamplesUntilStartProcess <= bufferSize,
                "Expected this value to be <= bufferSize. We are truncating a long long into an int. It is only safe to do so when it is <= bufferSize");
            processIndex = static_cast<int>(m_numSamplesUntilStartProcess);
            m_numSamplesUntilStartProcess = 0;

            returnValue = true;
            *outProcessIndex = processIndex;
        }
        else
        {
            m_numSamplesUntilStartProcess -= bufferSize;
        }
    }

    return returnValue;
}

bool rf::Fader::Process_Part2_ApplyTheFade(MixItem* item, int bufferSize, int processIndex)
{
    if (m_state == State::UpdateAmplitude)
    {
        for (int i = processIndex; i < bufferSize; ++i)
        {
            FadeData& data = m_processingFadeData;
            const float percent = data.m_fadeSampleCounter * m_pendingFadeData.m_durationInverse;
            m_currentAmplitude = (data.m_destinationAmplitude * percent) + (data.m_startAmplitude * (1.0f - percent));
            m_amplitudeBuffer[i] = m_currentAmplitude;
            data.m_fadeSampleCounter = std::min(data.m_fadeSampleCounter + 1, data.m_durationSamples);
        }

        if (m_processingFadeData.m_fadeSampleCounter == m_processingFadeData.m_durationSamples)
        {
            m_state = State::SetBufferToCurrentAmplitude;
        }
    }

    const int numChannels = item->m_channels;
    Buffer* sampleData = item->m_arrayOfChannels;
    for (int i = 0; i < numChannels; ++i)
    {
        sampleData[i].Multiply(m_amplitudeBuffer);
    }

    return m_state == State::UpdateAmplitude || m_state == State::Pending;
}

bool rf::Fader::Process(MixItem* item, int bufferSize)
{
    int processIndex;
    Process_Part1_DoesFadeStartThisBuffer(bufferSize, &processIndex);
    return Process_Part2_ApplyTheFade(item, bufferSize, processIndex);
}

float rf::Fader::GetAmplitude() const
{
    return m_currentAmplitude;
}

bool rf::Fader::GetIsFadeComplete() const
{
    return m_isFadeComplete;
}

bool rf::Fader::IsFading() const
{
    return m_state == State::UpdateAmplitude || m_state == State::Pending;
}