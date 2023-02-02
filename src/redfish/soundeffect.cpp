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

#include "soundeffect.h"

#include "assert.h"
#include "assetsystem.h"
#include "context.h"
#include "functions.h"
#include "playcommands.h"
#include "sync.h"

#define RF_COPY_SOUNDEFFECT                                                              \
    m_context = soundEffect.m_context;                                                   \
    m_commands = soundEffect.m_commands;                                                 \
    m_variations = soundEffect.m_variations;                                             \
    m_soundEffectHandle = soundEffect.m_soundEffectHandle;                               \
    m_mixGroup = soundEffect.m_mixGroup;                                                 \
    m_playbackRule = soundEffect.m_playbackRule;                                         \
    m_lastSelectedRoundRobin = soundEffect.m_lastSelectedRoundRobin;                     \
    m_smartShuffleHistoryIndex = soundEffect.m_smartShuffleHistoryIndex;                 \
    m_pitch = soundEffect.m_pitch;                                                       \
    m_amplitude = soundEffect.m_amplitude;                                               \
    m_isLooping = soundEffect.m_isLooping;                                               \
    for (int i = 0; i < k_maxHistorySize; ++i)                                           \
    {                                                                                    \
        m_smartShufflePlaybackHistory[i] = soundEffect.m_smartShufflePlaybackHistory[i]; \
    }

rf::SoundEffect::SoundEffect(Context* context)
    : m_context(context)
    , m_commands(&m_context->m_commandProcessor)
    , m_soundEffectHandle(CreateSoundEffectHandle())
    , m_mixGroup(context->GetMixerSystem()->GetMasterMixGroup())
{
    Reserve();
}

rf::SoundEffect::SoundEffect(const SoundEffect& soundEffect)
{
    Reserve();
    RF_COPY_SOUNDEFFECT
    m_soundEffectHandle = CreateSoundEffectHandle();
}

rf::SoundEffect::SoundEffect(SoundEffect&& soundEffect)
{
    Reserve();
    RF_COPY_SOUNDEFFECT
    m_soundEffectHandle = CreateSoundEffectHandle();

    soundEffect.m_context = nullptr;
    soundEffect.m_commands = nullptr;
    soundEffect.m_variations.clear();
    soundEffect.m_soundEffectHandle = SoundEffectHandle();
}

rf::SoundEffect& rf::SoundEffect::operator=(const SoundEffect& soundEffect)
{
    if (this != &soundEffect)
    {
        Free();
        Reserve();
        RF_COPY_SOUNDEFFECT
        m_soundEffectHandle = CreateSoundEffectHandle();
    }
    return *this;
}

rf::SoundEffect& rf::SoundEffect::operator=(SoundEffect&& soundEffect)
{
    if (this != &soundEffect)
    {
        Free();
        Reserve();
        RF_COPY_SOUNDEFFECT
        m_soundEffectHandle = CreateSoundEffectHandle();

        soundEffect.m_context = nullptr;
        soundEffect.m_commands = nullptr;
        soundEffect.m_variations.clear();
        soundEffect.m_soundEffectHandle = SoundEffectHandle();
    }
    return *this;
}

rf::SoundEffect::~SoundEffect()
{
    Free();
}

void rf::SoundEffect::Play()
{
    if (m_context->GetNumPlayingVoices() >= RF_MAX_VOICES)
    {
        return;
    }

    const Variation& variation = SelectVariation();
    const float variationPitch = Functions::RandomFloat(variation.m_minPitch, variation.m_maxPitch);
    const float minVariationAmp = Functions::DecibelToAmplitude(variation.m_minDb);
    const float maxVariationAmp = Functions::DecibelToAmplitude(variation.m_maxDb);
    const float variationAmp = Functions::RandomFloat(minVariationAmp, maxVariationAmp);

    AudioCommand cmd;
    PlayCommand& data = EncodeAudioCommand<PlayCommand>(&cmd);
    data.m_audioDataIndex = m_context->GetAssetSystem()->GetAudioDataIndex(variation.m_audioHandle);
    data.m_audioHandle = variation.m_audioHandle;
    data.m_soundEffectHandle = m_soundEffectHandle;
    data.m_playCount = m_isLooping ? 0 : 1;
    data.m_mixGroupHandle = m_mixGroup->GetMixGroupHandle();
    data.m_pitch = m_pitch * variationPitch;
    data.m_amplitude = m_amplitude * variationAmp;
    data.m_positioningParameters = m_positioningParamters;
    m_commands->Add(cmd);
}

void rf::SoundEffect::Stop()
{
    AudioCommand cmd;
    StopSoundEffectCommand& data = EncodeAudioCommand<StopSoundEffectCommand>(&cmd);
    data.m_soundEffectHandle = m_soundEffectHandle;
    m_commands->Add(cmd);
}

void rf::SoundEffect::Fade(float volumeDb, const Sync& sync, const Sync& duration)
{
    AudioCommand cmd;
    SoundEffectFadeCommondCommand& data = EncodeAudioCommand<SoundEffectFadeCommondCommand>(&cmd);
    data.m_soundEffectHandle = m_soundEffectHandle;
    data.m_sync = sync;
    data.m_duration = duration;
    data.m_amplitude = Functions::DecibelToAmplitude(volumeDb);
    m_commands->Add(cmd);
}

void rf::SoundEffect::FadeOutAndStop(const Sync& sync, const Sync& duration)
{
    AudioCommand cmd;
    SoundEffectFadeCommondCommand& data = EncodeAudioCommand<SoundEffectFadeCommondCommand>(&cmd);
    data.m_soundEffectHandle = m_soundEffectHandle;
    data.m_sync = sync;
    data.m_duration = duration;
    data.m_amplitude = 0.0f;
    data.m_stopOnDone = true;
    m_commands->Add(cmd);
}

void rf::SoundEffect::SetVolumeDb(float volumeDb)
{
    const float amplitude = Functions::DecibelToAmplitude(volumeDb);
    if (Functions::FloatEquality(m_amplitude, amplitude))
    {
        return;
    }

    m_amplitude = amplitude;

    AudioCommand cmd;
    SoundEffectAmplitudeCommand& data = EncodeAudioCommand<SoundEffectAmplitudeCommand>(&cmd);
    data.m_soundEffectHandle = m_soundEffectHandle;
    data.m_amplitude = m_amplitude;
    m_commands->Add(cmd);
}

float rf::SoundEffect::GetVolumeDb() const
{
    return Functions::AmplitudeToDecibel(m_amplitude);
}

void rf::SoundEffect::SetPitch(float pitch)
{
    if (Functions::FloatEquality(m_pitch, pitch))
    {
        return;
    }

    m_pitch = Functions::Clamp(pitch, 0.0f, 2.0f);

    AudioCommand cmd;
    SoundEffectPitchCommand& data = EncodeAudioCommand<SoundEffectPitchCommand>(&cmd);
    data.m_soundEffectHandle = m_soundEffectHandle;
    data.m_pitch = m_pitch;
    m_commands->Add(cmd);
}

float rf::SoundEffect::GetPitch() const
{
    return m_pitch;
}

rf::SoundEffect::Variation& rf::SoundEffect::AddVariation(AudioHandle audioHandle)
{
    Variation& variation = m_variations.emplace_back();
    variation.m_audioHandle = audioHandle;
    return variation;
}

void rf::SoundEffect::SetPlaybackRule(PlaybackRule playbackRule)
{
    m_playbackRule = playbackRule;
}

rf::SoundEffect::PlaybackRule rf::SoundEffect::GetPlaybackRule() const
{
    return m_playbackRule;
}

void rf::SoundEffect::SetPositioningParameters(const PositioningParameters& positioningParameters)
{
    if (m_positioningParamters == positioningParameters)
    {
        return;
    }

    m_positioningParamters = positioningParameters;
    m_positioningParamters.m_panAngle = Functions::Clamp(m_positioningParamters.m_panAngle, -1.0f, 1.0f);

    AudioCommand cmd;
    SoundEffectPositioningParamtersCommand& data = EncodeAudioCommand<SoundEffectPositioningParamtersCommand>(&cmd);
    data.m_soundEffectHandle = m_soundEffectHandle;
    data.m_positioningParameters = m_positioningParamters;
    m_commands->Add(cmd);
}

const rf::PositioningParameters& rf::SoundEffect::GetPositioningParameters() const
{
    return m_positioningParamters;
}

void rf::SoundEffect::SetIsLooping(bool isLooping)
{
    m_isLooping = isLooping;
}

bool rf::SoundEffect::GetIsLooping() const
{
    return m_isLooping;
}

void rf::SoundEffect::SetMixGroup(MixGroup* mixGroup)
{
    m_mixGroup = mixGroup;
}

rf::MixGroup* rf::SoundEffect::GetMixGroup() const
{
    return m_mixGroup;
}

const rf::SoundEffect::Variation& rf::SoundEffect::SelectVariation()
{
    const int numVariations = static_cast<int>(m_variations.size());
    int index = 0;

    switch (m_playbackRule)
    {
        case PlaybackRule::SmartShuffle:
        {
            if (numVariations == 1)
            {
                break;
            }
            else
            {
                int shuffleIndex = Functions::RandomInt(0, numVariations - 1);

                while (FoundIndexInSmartShuffleHistory(shuffleIndex))
                {
                    shuffleIndex = Functions::RandomInt(0, numVariations - 1);
                }

                m_smartShufflePlaybackHistory[m_smartShuffleHistoryIndex] = shuffleIndex;
                const int maxSmartShuffleHistory = static_cast<int>(floor(numVariations * 0.5f));
                m_smartShuffleHistoryIndex = (m_smartShuffleHistoryIndex + 1) % maxSmartShuffleHistory;
                index = shuffleIndex;
            }
            break;
        }
        case PlaybackRule::RoundRobin:
        {
            const int tempIndex = m_lastSelectedRoundRobin;
            m_lastSelectedRoundRobin = (m_lastSelectedRoundRobin + 1) % numVariations;
            index = tempIndex;
        }
        case PlaybackRule::Random:
        {
            index = Functions::RandomInt(0, numVariations - 1);
        }
        default: break;
    }

    return m_variations[index];
}

bool rf::SoundEffect::FoundIndexInSmartShuffleHistory(int index) const
{
    const int maxSmartShuffleHistory = static_cast<int>(floor(m_variations.size() * 0.5f));
    for (int i = 0; i < maxSmartShuffleHistory; ++i)
    {
        if (m_smartShufflePlaybackHistory[i] == index)
        {
            return true;
        }
    }

    return false;
}

void rf::SoundEffect::Reserve()
{
    m_variations.reserve(8);
}

void rf::SoundEffect::Free()
{
    m_variations.~vector();
}

rf::SoundEffect::Variation& rf::SoundEffect::Variation::SetMinVolumeDb(float volumeDb)
{
    m_minDb = volumeDb;
    return *this;
}

rf::SoundEffect::Variation& rf::SoundEffect::Variation::SetMaxVolumeDb(float volumeDb)
{
    m_maxDb = volumeDb;
    return *this;
}

rf::SoundEffect::Variation& rf::SoundEffect::Variation::SetMinPitch(float pitch)
{
    m_minPitch = pitch;
    return *this;
}

rf::SoundEffect::Variation& rf::SoundEffect::Variation::SetMaxPitch(float pitch)
{
    m_maxPitch = pitch;
    return *this;
}

#undef RF_COPY_SOUNDEFFECT