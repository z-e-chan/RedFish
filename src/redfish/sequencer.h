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
#include "fader.h"
#include "layerset.h"
#include "mixitem.h"
#include "stingerset.h"
#include "transitioncondition.h"

namespace rf
{
class Conductor;
class Messenger;
class MusicDatabase;
struct AudioData;

class Sequencer
{
public:
    Sequencer(const MusicDatabase* musicDatabase, const AudioSpec& spec, Messenger* messanger);
    Sequencer(const Sequencer&) = delete;
    Sequencer(Sequencer&&) = delete;
    Sequencer& operator=(const Sequencer&) = delete;
    Sequencer& operator=(Sequencer&&) = delete;
    ~Sequencer() = default;

    enum class Result
    {
        None,
        Stop,
    };

    Result Process(Conductor* conductor, long long playhead, MixItem* outMixItems, int* outstartIndex, const AudioData** audioData);
    void Stop(long long stopTime, long long playhead);
    void Fade(long long startTime, float amplitude, int sampleDuration, long long playhead, bool stopOnDone);
    void Reset(bool resetStingers);
    void AddTransition(int transitionIndex);
    bool IsPlaying() const;
    bool IsProcessingTransition() const;
    const MusicTransitionRequest& GetCurrentTransition() const;

    bool Unload(AudioHandle audioHandle, long long playhead);
    void DestroyCue(int cueIndex, long long playhead);
    void DestroyTransition(int transitionIndex, long long playhead);
    void DestroyStinger(int stingerIndex, long long playhead);

private:
    enum class State
    {
        GetTransition,
        ProcessingTransition,
        FollowUp,
    };

    AudioSpec m_spec;
    const MusicDatabase* m_musicDatabase = nullptr;
    Messenger* m_messanger = nullptr;
    MusicTransitionRequest m_pendingTransition;
    MusicTransitionRequest m_currentTransition;
    int m_transitionIndices[RF_MAX_TRANSITIONS];
    Fader m_fader;
    MixItem m_transformationMixItem;
    int m_transitionInsertIndex = 0;
    int m_transitionGetIndex = 0;
    int m_numTransitions = 0;
    State m_state = State::GetTransition;
    State m_goToAfterFollowUp = State::GetTransition;
    LayerSet m_layerSet;
    StingerSet m_stingerSet;
    bool m_isStopping = false;
    bool m_stopOnDoneFade = false;

    bool GetRequest(long long playhead, Conductor* conductor, MusicTransitionRequest* outRequest, const AudioData** audioData);
    Result UpdateDSP(MixItem* outMixItems, int startIndex, int endIndex);
};
}  // namespace rf