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
#include <vector>

#include "identifiers.h"
#include "mixgroup.h"
#include "positioningparameters.h"

namespace rf
{
class Context;
struct Sync;

class SoundEffect
{
public:
    SoundEffect() = default;
    SoundEffect(Context* context);
    SoundEffect(const SoundEffect&);
    SoundEffect(SoundEffect&&);
    SoundEffect& operator=(const SoundEffect&);
    SoundEffect& operator=(SoundEffect&&);
    ~SoundEffect();

    enum class PlaybackRule
    {
        SmartShuffle,
        RoundRobin,
        Random
    };

    struct Variation
    {
        AudioHandle m_audioHandle;
        float m_minDb = 0.0f;
        float m_maxDb = 0.0;
        float m_minPitch = 1.0f;
        float m_maxPitch = 1.0f;

        Variation& SetMinVolumeDb(float volumeDb);
        Variation& SetMaxVolumeDb(float volumeDb);
        Variation& SetMinPitch(float pitch);
        Variation& SetMaxPitch(float pitch);
    };

    void Play(const rf::Sync sync);
    void Play();
    void Stop();
    void Fade(float volumeDb, const Sync& sync, const Sync& duration);
    void FadeOutAndStop(const Sync& sync, const Sync& duration);
    void SetVolumeDb(float volumeDb);
    float GetVolumeDb() const;
    void SetPitch(float pitch);
    float GetPitch() const;
    Variation& AddVariation(AudioHandle audioHandle);
    void SetPlaybackRule(PlaybackRule playbackRule);
    PlaybackRule GetPlaybackRule() const;
    void SetPositioningParameters(const PositioningParameters& positioningParameters);
    const PositioningParameters& GetPositioningParameters() const;
    void SetIsLooping(bool isLooping);
    bool GetIsLooping() const;
    void SetMixGroup(MixGroup* mixGroup);
    MixGroup* GetMixGroup() const;

private:
    static constexpr int k_maxHistorySize = 20;
    Context* m_context = nullptr;
    CommandProcessor* m_commands = nullptr;
    std::vector<Variation> m_variations;
    SoundEffectHandle m_soundEffectHandle;
    MixGroup* m_mixGroup = nullptr;
    PlaybackRule m_playbackRule = PlaybackRule::SmartShuffle;
    PositioningParameters m_positioningParamters;
    int m_smartShufflePlaybackHistory[k_maxHistorySize];
    int m_lastSelectedRoundRobin = 0;
    int m_smartShuffleHistoryIndex = 0;
    float m_pitch = 1.0f;
    float m_amplitude = 1.0f;
    bool m_isLooping = false;

    const Variation& SelectVariation();
    bool FoundIndexInSmartShuffleHistory(int index) const;
    void Reserve();
    void Free();
};
}  // namespace rf