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
#include "conductor.h"
#include "nonallocatinglist.h"
#include "sequencer.h"

namespace rf
{
class AudioTimeline;
class MusicDatabase;
struct AudioSpec;

class MusicManager
{
public:
    MusicManager(AudioTimeline* timeline, const AudioSpec& spec);
    MusicManager(const MusicManager&) = delete;
    MusicManager(MusicManager&&) = delete;
    MusicManager& operator=(const MusicManager&) = delete;
    MusicManager& operator=(MusicManager&&) = delete;
    ~MusicManager();

    void Process(long long playhead, MixItem* outMixItems, int* outStartIndex);
    void Stop(long long stopTime, long long playhead);
    void Fade(long long startTime, float amplitude, int sampleDuration, long long playhead, bool stopOnDone);
    void AddTransition(int transitionIndex);
    void Unload(AudioHandle audioHandle, long long playhead);
    MusicDatabase* GetMusicDatabase();
    const MusicDatabase* GetMusicDatabase() const;
    long long CalculateStartTime(const Sync& sync, long long playhead) const;
    int GetSyncSamples(const Sync& sync) const;
    bool IsPlaying() const;
    CueHandle GetCurrentlyPlayingCueHandle() const;
    void PickAndAddTransition(const void* userData);

    void DestroyCue(int cueIndex, long long playhead);
    void DestroyTransition(int transitionIndex, long long playhead);
    void DestroyStinger(int stingerIndex, long long playhead);

private:
    AudioTimeline* m_timeline = nullptr;
    MusicDatabase* m_musicDatabase = nullptr;
    Conductor m_conductor;
    Sequencer m_sequencer;
    MusicTransitionRequest m_lastTransition;
    NonAllocatingList<int> m_cuesToDestory;
    NonAllocatingList<int> m_stingersToDestory;
    NonAllocatingList<int> m_transitionsToDestory;

    void DeleteFromMusicDatabase();
};
}  // namespace rf