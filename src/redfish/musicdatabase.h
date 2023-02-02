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
#include "layer.h"
#include "meter.h"
#include "sync.h"

namespace rf
{
class Cue;
class Messenger;
class Stinger;
class Transition;
struct TransitionCondition;

class MusicDatabase
{
public:
    MusicDatabase();
    MusicDatabase(const MusicDatabase&) = delete;
    MusicDatabase(MusicDatabase&&) = delete;
    MusicDatabase& operator=(const MusicDatabase&) = delete;
    MusicDatabase& operator=(MusicDatabase&&) = delete;
    ~MusicDatabase();

    struct CueData
    {
        Layer m_layers[RF_MAX_CUE_LAYERS];
        int m_numLayers = 0;
        CueHandle m_cueHandle;
        Meter m_meter;
        float m_tempo = 0.0f;
        float m_gainDb = 0.0f;
    };

    struct TransitionData
    {
        TransitionHandle m_transitionHandle;
        Sync m_sync;
        int m_followUpTransitionIndex = -1;
        int m_cueIndex = -1;
        int m_stingerIndex = -1;
        int m_playCount = 1;
        bool m_isLooping = false;
    };

    struct StingerData
    {
        StingerHandle m_stingerHandle;
        Sync m_sync;
        int m_cueIndex = -1;
        float m_gainDb = 0.0f;
    };

    void CreateCue(const Cue* cue, int index);
    void DestroyCue(int index, Messenger* messenger);
    const CueData& GetCueData(int index) const;

    void CreateTransition(const Transition* transition, int index);
    void DestroyTransition(int index, Messenger* messenger);
    const TransitionData& GetTransitionData(int index) const;
    int GetTransitionIndexThatMeetsCondition(const void* userData, CueHandle cueHandle, int currentBar, int currentBeat) const;

    void CreateStinger(const Stinger* stinger, int index);
    void DestroyStinger(int index, Messenger* messenger);
    const StingerData& GetStingerData(int index) const;

private:
    CueData* m_cueData = nullptr;
    TransitionData* m_transitionData = nullptr;
    TransitionHandle* m_transitionHandles = nullptr;
    TransitionCondition* m_transitionConditions = nullptr;
    StingerData* m_stingerData = nullptr;

    int GetCueDataIndexByHandle(CueHandle cueHandle) const;
};
}  // namespace rf