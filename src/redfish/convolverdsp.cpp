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

#include "convolverdsp.h"

#include "allocator.h"
#include "assert.h"
#include "audiodata.h"
#include "mixitem.h"

rf::ConvolverDSP::ConvolverDSP(const AudioSpec& spec)
    : DSPBase(spec)
{
    m_irAmplitudes = Allocator::AllocateArray<float>("IRAmps", PluginUtils::k_maxConvolverIRs);
    for (int i = 0; i < PluginUtils::k_maxConvolverIRs; ++i)
    {
        m_irAmplitudes[i] = 1.0f;
    }

    m_irs = Allocator::AllocateArray<AudioData>("IRs", PluginUtils::k_maxConvolverIRs);

    // Dry Signal Buffer
    {
        const int bufferSize = spec.m_bufferSize;
        m_dryBuffer = static_cast<float**>(Allocator::s_allocate(sizeof(float*) * PluginUtils::k_maxChannels, "DryBuffer", alignof(void*)));

        for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
        {
            m_dryBuffer[i] = static_cast<float*>(Allocator::s_allocate(sizeof(float) * bufferSize, "DryBuffer", alignof(float)));
            memset(m_dryBuffer[i], 0, sizeof(float) * bufferSize);
        }
    }
}

rf::ConvolverDSP::~ConvolverDSP()
{
    Allocator::DeallocateArray<float>(&m_irAmplitudes, PluginUtils::k_maxConvolverIRs);

    // Dry Signal Buffer
    {
        for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
        {
            Allocator::s_deallocate(m_dryBuffer[i]);
        }

        Allocator::s_deallocate(m_dryBuffer);
    }

    if (m_loaded)
    {
        for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
        {
            for (int j = 0; j < PluginUtils::k_maxConvolverIRs; ++j)
            {
                Allocator::s_deallocate(m_irArray[i][j]);
            }
            Allocator::s_deallocate(m_irArray[i]);
        }
        Allocator::s_deallocate(m_irArray);
    }

    for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
    {
        m_channelConvolvers[i].reset();
    }

    for (int i = 0; i < PluginUtils::k_maxConvolverIRs; ++i)
    {
        m_irs[i].Free();
    }
    Allocator::DeallocateArray<AudioData>(&m_irs, PluginUtils::k_maxConvolverIRs);
}

void rf::ConvolverDSP::LoadIR(const AudioData* ir, float amplitude, int index)
{
    if (!IndexCheck(index))
    {
        return;
    }

    if (ir->m_numChannels != PluginUtils::k_maxChannels)
    {
        RF_FAIL("Incorrect impulse channel count for impulse response. Impulse response not loaded.");
        return;
    }

    if (m_numIrs >= PluginUtils::k_maxConvolverIRs)
    {
        RF_FAIL("Too many impulse responses loaded.");
        return;
    }

    // Deep-copy the IR audio asset so it is immune to the IR being unloaded.
    m_irs[index].Allocate(*ir);
    ++m_numIrs;

    // Length of the IR that we are loading.
    const int lenIr = ir->m_numFrames;

    // If we are loading an IR with a greater length, we must reallocate.
    bool reallocate = false;

    if (lenIr > m_maxIrLen)
    {
        m_maxIrLen = lenIr;
        reallocate = true;
    }

    // Allocate the IR array.

    // [
    // channel 1: [[IR1], [IR2], [IR3]],
    // channel 2: [[IR1], [IR2], [IR3]]
    // ]

    // We allocate the IR array the first time.
    if (!m_loaded)
    {
        m_irArray = static_cast<float***>(Allocator::s_allocate(sizeof(float**) * PluginUtils::k_maxChannels, "IRArray", alignof(void*)));

        for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
        {
            m_irArray[i] = static_cast<float**>(Allocator::s_allocate(sizeof(float*) * PluginUtils::k_maxConvolverIRs, "IRArray", alignof(void*)));

            for (int j = 0; j < PluginUtils::k_maxConvolverIRs; ++j)
            {
                m_irArray[i][j] = static_cast<float*>(Allocator::s_allocate(sizeof(float) * m_maxIrLen, "IRArray", alignof(float)));
                memset(m_irArray[i][j], 0, sizeof(float) * m_maxIrLen);
            }
        }
    }
    // If we need to reallocate due to an increase in IR length.
    else if (reallocate && m_loaded)
    {
        for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
        {
            for (int j = 0; j < PluginUtils::k_maxConvolverIRs; ++j)
            {
                Allocator::s_deallocate(m_irArray[i][j]);
                m_irArray[i][j] = static_cast<float*>(Allocator::s_allocate(sizeof(float) * m_maxIrLen, "IRArray", alignof(float)));
                memset(m_irArray[i][j], 0, sizeof(float) * m_maxIrLen);
            }
        }
    }

    for (int k = 0; k < PluginUtils::k_maxConvolverIRs; ++k)
    {
        if (m_irs[k].m_numFrames > 0)
        {
            for (int i = 0; i < m_irs[k].m_numFrames; ++i)
            {
                float** samples = m_irs[k].m_arrayOfChannels;
                for (int j = 0; j < PluginUtils::k_maxChannels; ++j)
                {
                    m_irArray[j][k][i] = samples[j][i];
                }
            }
        }
        else
        {
            for (int i = 0; i < m_maxIrLen; ++i)
            {
                for (int j = 0; j < PluginUtils::k_maxChannels; ++j)
                {
                    m_irArray[j][k][i] = 0.0f;
                }
            }
        }
    }

    InitChannelConvolvers();
    SetIRAmplitude(amplitude, index);

    m_loaded = true;
}

void rf::ConvolverDSP::UnloadIR(int index)
{
    if (!IndexCheck(index))
    {
        return;
    }

    m_irs[index].Free();

    for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
    {
        for (int j = 0; j < m_maxIrLen; ++j)
        {
            m_irArray[i][index][j] = 0.0f;
        }
    }

    InitChannelConvolvers();

    --m_numIrs;
    if (m_numIrs == 0)
    {
        m_loaded = false;

        for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
        {
            for (int j = 0; j < PluginUtils::k_maxConvolverIRs; ++j)
            {
                Allocator::s_deallocate(m_irArray[i][j]);
            }
            Allocator::s_deallocate(m_irArray[i]);
        }
        Allocator::s_deallocate(m_irArray);
    }
}

void rf::ConvolverDSP::SetIRAmplitude(float amplitude, int index)
{
    if (!IndexCheck(index))
    {
        return;
    }

    m_irAmplitudes[index] = amplitude;
    UpdateAmplitudes();
}

void rf::ConvolverDSP::SetWetPercentage(float wetPercent)
{
    m_destinationWetPercentage = wetPercent;
}

void rf::ConvolverDSP::Process(MixItem* mixItem, int bufferSize)
{
    if (m_bypass || !m_loaded || !m_dryBuffer)
    {
        return;
    }

    // Store the mix item buffer into the dry buffer.
    const int numChannels = mixItem->m_channels;
    for (int i = 0; i < numChannels; ++i)
    {
        for (int j = 0; j < bufferSize; ++j)
        {
            m_dryBuffer[i][j] = mixItem->m_arrayOfChannels[i][j];
        }
    }

    for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
    {
        float* buffer = mixItem->m_arrayOfChannels[i].GetAsFloatBuffer();
        m_channelConvolvers[i].process(buffer, buffer, bufferSize);
    }

    // Apply the wet/dry ratio on the mix item.
    const float inverse = 1.0f / (bufferSize - 1);

    for (int i = 0; i < numChannels; ++i)
    {
        Buffer* buffer = mixItem->m_arrayOfChannels;
        for (int j = 0; j < bufferSize; ++j)
        {
            const float percent = inverse * j;
            const float wet = (m_destinationWetPercentage * percent) + (m_startWetPercentage * (1.0f - percent));
            buffer[i][j] = (buffer[i][j] * wet) + (m_dryBuffer[i][j] * (1.0f - wet));
        }
    }

    m_startWetPercentage = m_destinationWetPercentage;
}

bool rf::ConvolverDSP::IndexCheck(int index)
{
    if (index >= 0 && index < PluginUtils::k_maxConvolverIRs)
    {
        return true;
    }

    RF_FAIL("Index out of bounds");
    return false;
}

void rf::ConvolverDSP::InitChannelConvolvers()
{
    for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
    {
        m_channelConvolvers[i].init(m_spec.m_bufferSize, m_irArray[i], m_maxIrLen);
    }

    UpdateAmplitudes();
}

void rf::ConvolverDSP::UpdateAmplitudes()
{
    for (int i = 0; i < PluginUtils::k_maxChannels; ++i)
    {
        m_channelConvolvers[i].setImpulseResponseAmplitudes(m_irAmplitudes);
    }
}