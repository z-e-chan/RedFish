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
#include "sync.h"
#include "transitioncondition.h"

namespace rf
{
class Cue;
class Stinger;
class Transition;

struct CreateCueCommand
{
    int m_index = -1;
    const Cue* m_cue = nullptr;
    static AudioCommandCallback s_callback;
};

struct DestroyCueCommand
{
    int m_index = -1;
    static AudioCommandCallback s_callback;
};

struct CreateTransitionCommand
{
    int m_index = -1;
    const Transition* m_transition = nullptr;
    static AudioCommandCallback s_callback;
};

struct DestroyTransitionCommand
{
    int m_index = -1;
    static AudioCommandCallback s_callback;
};

struct CreateStingerCommand
{
    int m_index = -1;
    const Stinger* m_stinger = nullptr;
    static AudioCommandCallback s_callback;
};

struct DestroyStingerCommand
{
    int m_index = -1;
    static AudioCommandCallback s_callback;
};

struct PlayTransitionCommand
{
    int m_index = -1;
    static AudioCommandCallback s_callback;
};

struct PickTransitionCommand
{
    uint8_t m_userData[TransitionCondition::k_dataSize] = {};
    static AudioCommandCallback s_callback;
};

struct StopMusicCommand
{
    Sync m_sync;
    static AudioCommandCallback s_callback;
};

struct FadeOutAndStopMusicCommand
{
    Sync m_sync;
    Sync m_duration;
    static AudioCommandCallback s_callback;
};

struct PlayStingerCommand
{
    int m_index = -1;
    static AudioCommandCallback s_callback;
};
}  // namespace rf