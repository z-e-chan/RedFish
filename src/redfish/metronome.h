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
#include "beatcalculator.h"

namespace rf
{
class Messenger;
class MusicDatabase;
struct MusicTransitionRequest;

class Metronome
{
public:
    Metronome(const AudioSpec& spec, const MusicDatabase* musicDatabase, Messenger* messanger);

    void Set(float tempo, const Meter& meter);
    void Update(long long playhead, const MusicTransitionRequest& request, bool isPlaying);
    void Reset();
    static int GetSyncSamples(const AudioSpec& spec, const Sync& sync, float tempo, const Meter& meter);
    double PreciseBeatSwitch(const Sync& sync) const;
    float GetTempo() const;
    Meter GetMeter() const;
    int GetBarCounter() const;
    int GetBeatCounter() const;

private:
    AudioSpec m_spec;
    BeatCalculator m_beatCalculator;
    Messenger* m_messanger = nullptr;
    const MusicDatabase* m_musicDatabase = nullptr;
    Meter m_meter;
    float m_tempo = -1.0f;
    double m_samplesPerBeat = 0.0;
    int m_barCounter = 0;
    int m_beatCounter = 0;
};
}  // namespace rf