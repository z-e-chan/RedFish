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
#include "gain.h"
#include "identifiers.h"

namespace rf
{
class Messenger;
struct AudioData;
struct MixItem;

static constexpr int k_stopSamples = 32;

class BaseVoice
{
public:
    BaseVoice(int bufferSize);

    struct PlayParams
    {
        AudioHandle m_audioHandle;
        SoundEffectHandle m_soundEffectHandle;
        StingerHandle m_stingerHandle;
        MixGroupHandle m_mixGroupHandle;
        const AudioData* m_audioData = nullptr;
        long long m_startTime = 0;
        float m_amplitude = 1.0f;
        float m_pitch = 1.0f;
        int m_playCount = 1;
    };

    struct Info
    {
        AudioHandle m_audioHandle;
        int m_lastFilledFrame = -1;
        bool m_mixItemFullyFilled = false;
        bool m_done = false;
        bool m_looped = false;
        bool m_started = false;
        bool m_stopped = false;

        bool operator==(const Info& other) const;
    };

    void PlayBase(const PlayParams& params);
    Info FillMixItemBase(long long playhead, MixItem* mixItem, int startingIndex, int fillSize, int bufferSize, Messenger* messenger);
    void ResetBase(Messenger* messenger);
    bool IsPlaying() const;
    AudioHandle GetAudioHandle() const;
    SoundEffectHandle GetSoundEffectHandle() const;
    MixGroupHandle GetMixGroupHandle() const;
    StingerHandle GetStingerHandle() const;
    long long GetStartTime() const;

protected:
    long long m_startTime = -1;
    Gain m_gain;
    AudioHandle m_audioHandle;
    SoundEffectHandle m_soundEffectHandle;
    MixGroupHandle m_mixGroupHandle;
    StingerHandle m_stingerHandle;
    float** m_arrayOfChannels = nullptr;
    float m_pitch = 1.0f;
    int m_channels = 0;
    int m_seek = 0;
    int m_numFrames = 0;
    int m_localPlayCount = 0;
    int m_playCount = 0;
    bool m_isPlaying = false;
};
}  // namespace rf