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

#pragma once
#include <external/fftconvolver/FFTConvolver.h>

#include "buffer.h"
#include "dspbase.h"
#include "pluginutils.h"

namespace rf
{
struct AudioData;

class ConvolverDSP : public DSPBase
{
public:
    ConvolverDSP(const AudioSpec& spec);
    ConvolverDSP(const ConvolverDSP&) = delete;
    ConvolverDSP(ConvolverDSP&&) = delete;
    ConvolverDSP& operator=(const ConvolverDSP&) = delete;
    ConvolverDSP& operator=(ConvolverDSP&&) = delete;
    ~ConvolverDSP();

    void LoadIR(const AudioData* ir, float amplitude, int index);
    void UnloadIR(int index);
    void SetIRAmplitude(float amplitude, int index);
    void SetWetPercentage(float wetPercent);
    void Process(MixItem* mixItem, int bufferSize) override final;
    static bool IndexCheck(int index);

private:
    fftconvolver::FFTConvolver m_channelConvolvers[PluginUtils::k_maxChannels];
    AudioData* m_irs = nullptr;
    float*** m_irArray = nullptr;
    float** m_dryBuffer = nullptr;
    float* m_irAmplitudes = nullptr;
    int m_numIrs = 0;
    int m_maxIrLen = 0;
    float m_startWetPercentage = 1.0f;
    float m_destinationWetPercentage = 1.0f;
    bool m_loaded = false;

    void InitChannelConvolvers();
    void UpdateAmplitudes();
};
}  // namespace rf