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
#include "defines.h"
#include "identifiers.h"

namespace rf
{
struct MixGroupState
{
    MixGroupHandle m_mixGroupHandle;
    MixGroupHandle m_outputMixGroupHandle;
    int m_sendSlots[RF_MAX_MIX_GROUP_SENDS];
    int m_pluginSlots[RF_MAX_MIX_GROUP_PLUGINS];
    float m_peakAmplitude = 0.0f;
    float m_priority = 0.0f;
    float m_volumeDb = 0.0f;
    bool m_isMaster = false;

    MixGroupState();
};
}  // namespace rf