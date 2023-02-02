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
#include "fader.h"
#include "positioningdsp.h"

namespace rf
{
struct AudioData;
struct AudioSpec;
struct Layer;
struct PlayCommand;
struct StingerData;

class Voice final : public BaseVoice
{
public:
    Voice(const AudioSpec& spec);

    void Play(const AudioData* audioData, const PlayCommand& command, long long startTime);
    void Play(const AudioData* audioData, const Layer& layer, StingerHandle stingerHandle, long long startTime, float amplitude);
    void Stop(long long stopTime, long long playhead);
    void Fade(long long startTime, float amplitude, int sampleDuration, long long playhead, bool stopOnDone);
    void SetAmplitude(float amplitude);
    void SetPitch(float pitch);
    void SetPosition(const PositioningParameters& positioningParameters, bool interpolate);
    BaseVoice::Info FillMixItem(long long playhead, MixItem* outMixItem, int bufferSize, Messenger* messenger);
    void Reset(Messenger* messenger);

private:
    Fader m_fader;
    PositioningDSP m_positioning;
    bool m_isStopping = false;
    bool m_stopOnDoneFade = false;

    enum class Result
    {
        None,
        Stop,
    };

    Result UpdateDSP(MixItem* mixItem, int bufferSize);
};
}  // namespace rf