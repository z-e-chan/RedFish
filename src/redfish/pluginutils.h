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

namespace rf::PluginUtils
{
static constexpr int k_maxChannels = 2;
static constexpr int k_maxConvolverIRs = 3;
static constexpr float k_maxDelayTime = 5000.0f;
static constexpr float k_maxDelayFeedback = 1.0f;
static constexpr float k_minDelayFeedback = 0.0f;
static constexpr int k_maxFilterDelayLines = 2;
static constexpr float k_maxFilterCutoff = 20000.0f;
static constexpr float k_minFilterCutoff = 20.0f;
static constexpr float k_maxFilterQ = 1000.0f;
static constexpr float k_minFilterQ = 0.1f;
static float k_piOverTwo = 1.57079632679489661923f;
static float k_twoOverPi = 0.63661977236758134307f;
static float k_twoPi = 6.28318530717958647692f;
static float k_sqrtTwo = 1.41421356237309504880f;

#define RF_SEND_PLUGIN_CREATE_COMMAND(command)         \
    AudioCommand cmd;                                  \
    command& data = EncodeAudioCommand<command>(&cmd); \
    data.m_mixGroupHandle = mixGroupHandle;            \
    data.m_dspIndex = pluginIndex;                     \
    data.m_mixGroupSlot = mixGroupSlot;                \
    m_commands->Add(cmd);
}  // namespace rf::PluginUtils