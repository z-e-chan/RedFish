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
#include "basevoice.h"
#include "musicdatabase.h"
#include "musictransitionrequest.h"

namespace rf
{
class Messenger;
class MusicVoice;
class RedFishContext;
struct AudioSpec;

class LayerSet
{
public:
    LayerSet(const AudioSpec& spec, Messenger* messanger);
    ~LayerSet();

    void Play(const MusicTransitionRequest& request,
              const MusicDatabase::TransitionData& transitionData,
              const MusicDatabase::CueData& cueData,
              const AudioData** audioData);
    void Reset();
    BaseVoice::Info Process(long long playhead, int startingIndex, int fillSize, bool forceVoicesToDone, MixItem* outMixItems, int* outNumMixItems);
    bool IsPlaying() const;

private:
    Messenger* m_messanger = nullptr;
    MusicVoice* m_voices = nullptr;
    int m_numLayers = 0;
    int m_bufferSize = 0;
};
}  // namespace rf