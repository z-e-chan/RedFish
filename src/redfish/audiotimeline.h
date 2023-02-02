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
#include "messenger.h"
#include "musicmanager.h"
#include "summingmixer.h"
#include "voiceset.h"

namespace rf
{
struct AudioData;
struct MixItem;

class AudioTimeline
{
public:
    AudioTimeline(int numChannels, int bufferSize, int sampleRate);
    AudioTimeline(const AudioTimeline&) = delete;
    AudioTimeline(AudioTimeline&&) = delete;
    AudioTimeline& operator=(const AudioTimeline&) = delete;
    AudioTimeline& operator=(AudioTimeline&&) = delete;
    ~AudioTimeline();

    void Process(float* buffer, int size);
    const long long& GetPlayhead() const;
    const AudioSpec& GetAudioSpec() const;
    const AudioData* GetAudioData(int index) const;
    void Shutdown();
    static int GetMaxNumMixItems();

    const AudioData** m_audioDataReferences = nullptr;
    AudioSpec m_spec;
    Messenger m_messenger;
    VoiceSet m_voiceSet;
    SummingMixer m_summingMixer;
    MusicManager m_musicManager;

private:
    MixItem* m_mixItems = nullptr;
    long long m_playhead = 0;
    int m_mixItemIndex = 0;

    enum class ShutdownState
    {
        None,
        Stop,
        Stopping,
        SendShutdownCompleteMessage,
        Complete,
    } m_shutdownState = ShutdownState::None;

    void HandleShutdown();
};
}  // namespace rf