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
#include "sync.h"
#include "transitioncondition.h"

namespace rf
{
class Cue;
class Stinger;
struct TransitionParameters;

class Transition
{
public:
    Transition(const TransitionParameters& parameters, int index);
    Transition(const Transition&) = delete;
    Transition(Transition&&) = delete;
    Transition& operator=(const Transition&) = delete;
    Transition& operator=(Transition&&) = delete;
    ~Transition() = default;

    explicit operator bool() const;

    TransitionHandle GetTransitionHandle() const;
    const TransitionCondition& GetTransitionCondition() const;
    const Sync& GetSync() const;
    const Cue* GetCue() const;
    const Transition* GetFollowUpTransition() const;
    const Stinger* GetStinger() const;
    const int GetPlayCount() const;
    bool IsLooping() const;
    const char* GetName() const;

private:
    char m_name[RF_MAX_NAME_SIZE] = {};
    TransitionHandle m_transitionHandle;
    TransitionCondition m_condition;
    Sync m_sync;
    const Cue* m_cue = nullptr;
    const Transition* m_followUpTransition = nullptr;
    const Stinger* m_stinger = nullptr;
    int m_playCount = 0;
    int m_index = -1;

    int GetIndex() const;

    friend class MusicSystem;
};

struct TransitionParameters
{
    TransitionCondition m_condition;
    Sync m_sync;
    const Cue* m_cue = nullptr;
    const Transition* m_followUpTransition = nullptr;
    const Stinger* m_stinger = nullptr;
    const char* m_name = nullptr;
    int m_playCount = 0;
};
}  // namespace rf