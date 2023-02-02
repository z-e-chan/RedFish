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

#include "mixitem.h"

#include "allocator.h"
#include "buffer.h"

rf::MixItem::MixItem(int channels, int bufferSize)
{
    Allocate(channels, bufferSize);
}

rf::MixItem::MixItem(const MixItem& mixItem)
{
    Allocate(mixItem.m_channels, mixItem.m_bufferSize);
    for (int i = 0; i < m_channels; ++i)
    {
        for (int j = 0; j < m_bufferSize; ++j)
        {
            m_arrayOfChannels[i][j] = mixItem.m_arrayOfChannels[i][j];
        }
    }
}

rf::MixItem::MixItem(MixItem&& mixItem)
{
    m_channels = mixItem.m_channels;
    m_bufferSize = mixItem.m_bufferSize;
    m_arrayOfChannels = mixItem.m_arrayOfChannels;

    mixItem.m_channels = 0;
    mixItem.m_bufferSize = 0;
    mixItem.m_arrayOfChannels = nullptr;
}

rf::MixItem& rf::MixItem::operator=(const MixItem& mixItem)
{
    if (this != &mixItem)
    {
        Allocate(mixItem.m_channels, mixItem.m_bufferSize);

        for (int i = 0; i < m_channels; ++i)
        {
            for (int j = 0; j < m_bufferSize; ++j)
            {
                m_arrayOfChannels[i][j] = mixItem.m_arrayOfChannels[i][j];
            }
        }
    }

    return *this;
}

rf::MixItem& rf::MixItem::operator=(MixItem&& mixItem)
{
    if (this != &mixItem)
    {
        m_channels = mixItem.m_channels;
        m_bufferSize = mixItem.m_bufferSize;
        m_arrayOfChannels = mixItem.m_arrayOfChannels;

        mixItem.m_channels = 0;
        mixItem.m_bufferSize = 0;
        mixItem.m_arrayOfChannels = nullptr;
    }

    return *this;
}

rf::MixItem::~MixItem()
{
    Free();
}

void rf::MixItem::Sum(const MixItem& item, float amplitude)
{
    for (int i = 0; i < m_channels; ++i)
    {
        m_arrayOfChannels[i].Sum(item.m_arrayOfChannels[i], amplitude);
    }
}

void rf::MixItem::Multiply(const MixItem& item)
{
    for (int i = 0; i < m_channels; ++i)
    {
        m_arrayOfChannels[i].Multiply(item.m_arrayOfChannels[i]);
    }
}

void rf::MixItem::ZeroOut()
{
    for (int i = 0; i < m_channels; ++i)
    {
        m_arrayOfChannels[i].ZeroOut();
    }
}

void rf::MixItem::Set(float value)
{
    for (int i = 0; i < m_channels; ++i)
    {
        m_arrayOfChannels[i].Set(value);
    }
}

void rf::MixItem::ToInterleavedBuffer(float* buffer)
{
    for (int channel = 0; channel < m_channels; ++channel)
    {
        int index = channel;
        for (int i = 0; i < m_bufferSize; ++i)
        {
            buffer[index] = m_arrayOfChannels[channel][i];
            index += m_channels;
        }
    }
}

float rf::MixItem::GetPeakAmplitude() const
{
    float max = 0.0f;
    for (int i = 0; i < m_channels; ++i)
    {
        const float channelMax = m_arrayOfChannels[i].GetAbsoluteMax();
        if (channelMax > max)
        {
            max = channelMax;
        }
    }
    return max;
}

float rf::MixItem::GetPeakAmplitudeForChannel(int channel) const
{
    return m_arrayOfChannels[channel].GetAbsoluteMax();
}

void rf::MixItem::Allocate(int channels, int bufferSize)
{
    Free();

    m_channels = channels;
    m_bufferSize = bufferSize;
    m_arrayOfChannels = Allocator::AllocateArray<Buffer>("MixItemArrayOfChannels", m_channels, bufferSize);

    ZeroOut();
}

void rf::MixItem::Free()
{
    if (m_arrayOfChannels)
    {
        Allocator::DeallocateArray<Buffer>(&m_arrayOfChannels, m_channels);
        m_bufferSize = 0;
        m_channels = 0;
    }
}