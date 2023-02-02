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
#include "audiospec.h"
#include "metronome.h"
#include "musictransitionrequest.h"

namespace rf
{
class Messenger;
class MusicDatabase;
struct AudioData;
struct AudioSpec;

class Conductor
{
public:
    Conductor(MusicDatabase* musicDatabase, const AudioSpec& spec, Messenger* messanger);
    Conductor(const Conductor&) = delete;
    Conductor(Conductor&&) = delete;
    Conductor& operator=(const Conductor&) = delete;
    Conductor& operator=(Conductor&&) = delete;
    ~Conductor() = default;

    void Update(long long playhead, const MusicTransitionRequest& currentRequest, bool isPlaying);
    void Reset();
    MusicTransitionRequest CreateRequest(int transitionIndex, long long playhead, bool isPlaying, const AudioData** audioData);
    long long CalculateStartTime(const Sync& sync, long long playhead, bool isPlaying) const;
    int GetSyncSamples(const Sync& sync) const;
    void UpdateLastCreatedRequest(const MusicTransitionRequest& request);
    const Metronome& GetMetronome() const;

private:
    AudioSpec m_spec;
    Metronome m_metronome;
    MusicTransitionRequest m_lastCreatedRequest;
    MusicDatabase* m_musicDatabase = nullptr;
};
}  // namespace rf