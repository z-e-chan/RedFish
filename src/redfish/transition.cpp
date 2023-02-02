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

#include "transition.h"

#include <string.h>

rf::Transition::Transition(const TransitionParameters& parameters, int index)
    : m_transitionHandle(CreateTransitionHandle())
    , m_condition(parameters.m_condition)
    , m_sync(parameters.m_sync)
    , m_cue(parameters.m_cue)
    , m_followUpTransition(parameters.m_followUpTransition)
    , m_stinger(parameters.m_stinger)
    , m_playCount(parameters.m_playCount)
    , m_index(index)
{
    if (parameters.m_name)
    {
        strcpy_s(m_name, parameters.m_name);
    }
}

rf::Transition::operator bool() const
{
    return static_cast<bool>(m_cue);
}

rf::TransitionHandle rf::Transition::GetTransitionHandle() const
{
    return m_transitionHandle;
}

const rf::TransitionCondition& rf::Transition::GetTransitionCondition() const
{
    return m_condition;
}

const rf::Sync& rf::Transition::GetSync() const
{
    return m_sync;
}

const rf::Cue* rf::Transition::GetCue() const
{
    return m_cue;
}

const rf::Transition* rf::Transition::GetFollowUpTransition() const
{
    return m_followUpTransition;
}

const rf::Stinger* rf::Transition::GetStinger() const
{
    return m_stinger;
}

const int rf::Transition::GetPlayCount() const
{
    return m_playCount;
}

bool rf::Transition::IsLooping() const
{
    return m_playCount == 0;
}

const char* rf::Transition::GetName() const
{
    return m_name;
}

int rf::Transition::GetIndex() const
{
    return m_index;
}