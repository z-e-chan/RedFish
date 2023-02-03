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
#include "audiocommand.h"
#include "mixgroupstate.h"
#include "sync.h"

namespace rf
{
struct CreateMixGroupCommand
{
    MixGroupState m_mixGroupState;
    static AudioCommandCallback s_callback;
};

struct SetMixGroupAmplitudeCommand
{
    float m_amplitude = 1.0f;
    MixGroupHandle m_mixGroupHandle;
    static AudioCommandCallback s_callback;
};

struct SetMixGroupOutputCommand
{
    float m_priority = 0.0f;
    MixGroupHandle m_mixGroupHandle;
    MixGroupHandle m_outputMixGroupHandle;
    static AudioCommandCallback s_callback;
};

struct CreateSendCommand
{
    int m_sendIndex = -1;
    int m_mixGroupSlot = -1;
    float m_priority = 0.0f;
    MixGroupHandle m_mixGroupHandle;
    MixGroupHandle m_sendToMixGroupHandle;
    static AudioCommandCallback s_callback;
};

struct DeleteSendCommand
{
    int m_sendIndex = -1;
    int m_mixGroupSlot = -1;
    float m_priority = 0.0f;
    MixGroupHandle m_mixGroupHandle;
    static AudioCommandCallback s_callback;
};

struct SetSendAmplitudeCommand
{
    int m_sendIndex = -1;
    float m_amplitude = 1.0f;
    static AudioCommandCallback s_callback;
};

struct FadeMixGroupsCommand
{
    int m_stingerIndex = -1;
    int m_mixGroupIndices[RF_MAX_CUE_LAYERS];
    int m_numMixGroups = 0;
    float m_amplitude = 0.0f;
    Sync m_sync;
    Sync m_duration;
    static AudioCommandCallback s_callback;
};
}  // namespace rf