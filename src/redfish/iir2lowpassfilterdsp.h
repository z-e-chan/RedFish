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
#include "dspbase.h"
#include "pluginutils.h"

namespace rf
{
class IIR2LowpassFilterDSP : public DSPBase
{
public:
    IIR2LowpassFilterDSP(const AudioSpec& spec);

    void SetQ(float q);
    void SetCutoff(float cutoff);
    void Process(MixItem* mixItem, int bufferSize) override final;

private:
    float m_startQ = PluginUtils::k_minFilterQ;
    float m_destinationQ = PluginUtils::k_minFilterQ;
    float m_startCutoff = PluginUtils::k_maxFilterCutoff;
    float m_destinationCutoff = PluginUtils::k_maxFilterCutoff;

    float m_inputDelay[PluginUtils::k_maxChannels][PluginUtils::k_maxFilterDelayLines];
    float m_outputDelay[PluginUtils::k_maxChannels][PluginUtils::k_maxFilterDelayLines];

    void ResetDelayLines();
};
}  // namespace rf