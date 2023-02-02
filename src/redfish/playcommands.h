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
#include "identifiers.h"
#include "positioningparameters.h"
#include "sync.h"

namespace rf
{
struct PlayCommand
{
    AudioHandle m_audioHandle;
    SoundEffectHandle m_soundEffectHandle;
    MixGroupHandle m_mixGroupHandle;
    PositioningParameters m_positioningParameters;
    int m_audioDataIndex = -1;
    int m_playCount = 1;
    float m_amplitude = 1.0f;
    float m_pitch = 1.0f;
    static AudioCommandCallback s_callback;
};

struct StopSoundEffectCommand
{
    SoundEffectHandle m_soundEffectHandle;
    static AudioCommandCallback s_callback;
};

struct SoundEffectAmplitudeCommand
{
    SoundEffectHandle m_soundEffectHandle;
    float m_amplitude = 1.0f;
    static AudioCommandCallback s_callback;
};

struct SoundEffectPitchCommand
{
    SoundEffectHandle m_soundEffectHandle;
    float m_pitch = 1.0f;
    static AudioCommandCallback s_callback;
};

struct SoundEffectPositioningParamtersCommand
{
    SoundEffectHandle m_soundEffectHandle;
    PositioningParameters m_positioningParameters;
    static AudioCommandCallback s_callback;
};

struct SoundEffectFadeCommondCommand
{
    SoundEffectHandle m_soundEffectHandle;
    Sync m_sync;
    Sync m_duration;
    float m_amplitude = 1.0f;
    bool m_stopOnDone = false;
    static AudioCommandCallback s_callback;
};
}  // namespace rf