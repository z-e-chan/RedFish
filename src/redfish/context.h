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

#include "audiospec.h"
#include "commandprocessor.h"
#include "config.h"
#include "playingsoundinfo.h"

namespace rf
{
class AssetSystem;
class AudioCallback;
class AudioTimeline;
class MixerSystem;
class MusicSystem;
struct AudioData;

class Context
{
public:
    Context(const Config& config);
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
    ~Context();

    void Update();
    AssetSystem* GetAssetSystem();
    MixerSystem* GetMixerSystem();
    MusicSystem* GetMusicSystem();
    const AudioSpec& GetAudioSpec() const;
    int GetNumPlayingVoices() const;
    const std::vector<PlayingSoundInfo>& GetPlayingSoundInfo() const;

private:
    AudioSpec m_spec;
    Config m_config;
    CommandProcessor m_commandProcessor;
    std::vector<PlayingSoundInfo> m_playingSoundInfo;
    AudioTimeline* m_timeline = nullptr;
    AssetSystem* m_assetSystem = nullptr;
    MixerSystem* m_mixerSystem = nullptr;
    MusicSystem* m_musicSystem = nullptr;
    AudioCallback* m_audioCallback = nullptr;
    int m_numPlayingVoices = 0;

    void OnAudioCallback(float* buffer, int size);
    void SetAudioCallback(AudioCallback* audioCallback);

    friend class AudioCallback;
    friend class SoundEffect;
};
}  // namespace rf