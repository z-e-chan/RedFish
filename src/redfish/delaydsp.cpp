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

#include "delaydsp.h"

#include "allocator.h"
#include "functions.h"
#include "mixitem.h"
#include "pluginutils.h"

rf::DelayDSP::DelayDSP(const AudioSpec& spec)
    : DSPBase(spec)
{
    m_delayBufferSize = Functions::MsToSamples(PluginUtils::k_maxDelayTime, m_spec.m_sampleRate);
    m_buffer = Allocator::AllocateArray<Buffer>("DelayBuffer", spec.m_channels, m_delayBufferSize);
    m_feedbackBuffer = Allocator::AllocateArray<Buffer>("DelayFeedbackBuffer", spec.m_channels, m_delayBufferSize);
}

rf::DelayDSP::~DelayDSP()
{
    Allocator::DeallocateArray<Buffer>(&m_buffer, m_spec.m_channels);
    Allocator::DeallocateArray<Buffer>(&m_feedbackBuffer, m_spec.m_channels);
}

void rf::DelayDSP::SetDelay(int delay)
{
    m_delay = delay;
}

void rf::DelayDSP::SetFeedback(float feedback)
{
    m_feedback = feedback;
}

void rf::DelayDSP::Process(MixItem* mixItem, int bufferSize)
{
    if (m_bypass)
    {
        return;
    }

    Buffer* buffer = mixItem->m_arrayOfChannels;
    int writeHead = (m_readHead + m_delay) % m_delayBufferSize;

    for (int i = 0; i < bufferSize; ++i)
    {
        for (int c = 0; c < m_spec.m_channels; ++c)
        {
            m_buffer[c][writeHead] += buffer[c][i] + (m_feedback * m_feedbackBuffer[c][i]);
            buffer[c][i] = m_buffer[c][m_readHead];
            m_feedbackBuffer[c][i] = m_buffer[c][m_readHead];
            m_buffer[c][m_readHead] = 0.0f;
        }
        writeHead = (writeHead + 1) % m_delayBufferSize;
        m_readHead = (m_readHead + 1) % m_delayBufferSize;
    }
}