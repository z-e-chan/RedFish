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

#include "audiodata.h"

#include "allocator.h"

void rf::AudioData::Allocate(int numChannels, int numFrames, const float* sampleData)
{
    Free();

    m_numChannels = numChannels;
    m_numFrames = numFrames;
    m_numSamples = m_numChannels * m_numFrames;

    m_arrayOfChannels = Allocator::AllocateBytes<float*>("AudioDataArrayOfChannels", m_numChannels * sizeof(float*));
    for (int i = 0; i < m_numChannels; ++i)
    {
        m_arrayOfChannels[i] = Allocator::AllocateBytes<float>("AudioDataChannel", m_numFrames * sizeof(float));
    }

    int index = 0;
    for (int i = 0; i < m_numFrames; ++i)
    {
        for (int j = 0; j < m_numChannels; ++j)
        {
            m_arrayOfChannels[j][i] = sampleData[index++];
        }
    }
}

void rf::AudioData::Allocate(const AudioData& audioData)
{
    Free();

    m_numChannels = audioData.m_numChannels;
    m_numFrames = audioData.m_numFrames;
    m_numSamples = audioData.m_numSamples;

    m_arrayOfChannels = Allocator::AllocateBytes<float*>("AudioDataArrayOfChannels", m_numChannels * sizeof(float*));
    for (int i = 0; i < m_numChannels; ++i)
    {
        m_arrayOfChannels[i] = Allocator::AllocateBytes<float>("AudioDataChannel", m_numFrames * sizeof(float));
        memcpy(m_arrayOfChannels[i], audioData.m_arrayOfChannels[i], m_numFrames * sizeof(float));
    }
}

void rf::AudioData::Free()
{
    for (int i = 0; i < m_numChannels; ++i)
    {
        Allocator::DeallocateBytes(&m_arrayOfChannels[i]);
    }
    Allocator::DeallocateBytes(&m_arrayOfChannels);

    m_name = nullptr;
    m_arrayOfChannels = nullptr;
    m_numChannels = 0;
    m_numFrames = 0;
    m_numSamples = 0;
}
