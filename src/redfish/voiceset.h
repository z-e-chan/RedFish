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
#include "identifiers.h"
#include "musicdatabase.h"

namespace rf
{
class Messenger;
class Voice;
struct AudioData;
struct AudioSpec;
struct MixItem;
struct PlayCommand;
struct PositioningParameters;

class VoiceSet
{
public:
    VoiceSet(Messenger* messenger, const AudioSpec& spec);
    VoiceSet(const VoiceSet&) = delete;
    VoiceSet(VoiceSet&&) = delete;
    VoiceSet& operator=(const VoiceSet&) = delete;
    VoiceSet& operator=(VoiceSet&&) = delete;
    ~VoiceSet();

    void CreateVoice(const AudioData* audioData, const PlayCommand& command, long long startTime);
    void CreateVoice(const AudioData** audioData,
                     const MusicDatabase::StingerData& stingerData,
                     long long startTime,
                     const MusicDatabase* musicDatabase);
    void Process(long long playhead, MixItem* outMixItems, int* outNumMixItems);
    void Unload(AudioHandle audioHandle, long long playhead);
    void StopAll(long long stopTime, long long playhead);
    void StopBySoundEffectHandle(SoundEffectHandle soundEffectHandle, long long stopTime, long long playhead);
    void StopByStingerHandle(StingerHandle stingerHandle, long long stopTime, long long playhead);
    void StopByAudioHandle(AudioHandle audioHandle, long long stopTime, long long playhead);
    void StopIfIsStinger(long long stopTime, long long playhead);
    void Fade(SoundEffectHandle soundEffectHandle, long long startTime, float amplitude, int sampleDuration, long long playhead, bool stopOnDone);
    void SetAmplitudeBySoundEffectHandle(SoundEffectHandle soundEffectHandle, float amplitude);
    void SetPitchBySoundEffectHandle(SoundEffectHandle soundEffectHandle, float pitch);
    void SetPositionBySoundEffectHandle(SoundEffectHandle soundEffectHandle, const PositioningParameters& positioningParameters, bool interpolate);
    int GetNumVoices() const;

private:
    Voice* m_voices = nullptr;
    Messenger* m_messenger = nullptr;
    int m_bufferSize = 0;
    int m_numVoices = 0;
};
}  // namespace rf