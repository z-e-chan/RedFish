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

#include "voiceset.h"

#include "allocator.h"
#include "assert.h"
#include "audiospec.h"
#include "audiotimeline.h"
#include "functions.h"
#include "messenger.h"
#include "mixitem.h"
#include "voice.h"

rf::VoiceSet::VoiceSet(Messenger* messenger, const AudioSpec& spec)
    : m_messenger(messenger)
    , m_bufferSize(spec.m_bufferSize)
{
    m_voices = Allocator::AllocateArray<Voice>("VoiceSetVoices", RF_MAX_VOICES, spec);
}

rf::VoiceSet::~VoiceSet()
{
    Allocator::DeallocateArray<Voice>(&m_voices, RF_MAX_VOICES);
}

void rf::VoiceSet::CreateVoice(const AudioData* audioData, const PlayCommand& command, long long startTime)
{
    RF_ASSERT(m_numVoices < RF_MAX_VOICES, "Out of voices");
    if (m_numVoices < RF_MAX_VOICES)
    {
        m_voices[m_numVoices++].Play(audioData, command, startTime);
    }
}

void rf::VoiceSet::CreateVoice(const AudioData** audioData,
                               const MusicDatabase::StingerData& stingerData,
                               long long startTime,
                               const MusicDatabase* musicDatabase)
{
    const MusicDatabase::CueData& cueData = musicDatabase->GetCueData(stingerData.m_cueIndex);
    const int numLayers = cueData.m_numLayers;
    const int totalVoices = m_numVoices + cueData.m_numLayers;
    RF_ASSERT(totalVoices < RF_MAX_VOICES, "Out of voices");
    const float amplitude = Functions::DecibelToAmplitude(stingerData.m_gainDb);

    for (int i = 0; i < numLayers; ++i)
    {
        if (m_numVoices < RF_MAX_VOICES)
        {
            const Layer& layer = cueData.m_layers[i];
            const AudioData* audio = audioData[layer.m_audioDataIndex];
            m_voices[m_numVoices++].Play(audio, layer, stingerData.m_stingerHandle, startTime, amplitude);
        }
    }
}

void rf::VoiceSet::Process(long long playhead, MixItem* outMixItems, int* outNumMixItems)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        RF_ASSERT(i >= 0, "Expected positive i");
        RF_ASSERT(i < RF_MAX_VOICES, "i out of bounds");

        MixItem* item = &outMixItems[(*outNumMixItems)++];
        RF_ASSERT(*outNumMixItems < AudioTimeline::GetMaxNumMixItems(), "Too many mix items will be generated. Increase RF_MAX_VOICES");
        const BaseVoice::Info info = m_voices[i].FillMixItem(playhead, item, m_bufferSize, m_messenger);
        RF_ASSERT(item->m_mixGroupHandle, "Mix item has no mix group. This is incorrect.");

        if (info.m_done || info.m_stopped)
        {
            m_voices[i--] = m_voices[--m_numVoices];
        }
    }

    Message msg;
    msg.m_type = MessageType::ContextNumVoices;
    msg.GetContextNumVoicesData()->m_numVoices = m_numVoices;
    m_messenger->AddMessage(msg);
}

void rf::VoiceSet::Unload(AudioHandle audioHandle, long long playhead)
{
    StopByAudioHandle(audioHandle, playhead, playhead);
}

void rf::VoiceSet::StopAll(long long stopTime, long long playhead)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        m_voices[i].Stop(stopTime, playhead);
    }
}

void rf::VoiceSet::StopBySoundEffectHandle(SoundEffectHandle soundEffectHandle, long long stopTime, long long playhead)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetSoundEffectHandle() == soundEffectHandle)
        {
            m_voices[i].Stop(stopTime, playhead);
        }
    }
}

void rf::VoiceSet::StopByStingerHandle(StingerHandle stingerHandle, long long stopTime, long long playhead)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetStingerHandle() == stingerHandle)
        {
            m_voices[i].Stop(stopTime, playhead);
        }
    }
}

void rf::VoiceSet::StopByAudioHandle(AudioHandle audioHandle, long long stopTime, long long playhead)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetAudioHandle() == audioHandle)
        {
            m_voices[i].Stop(stopTime, playhead);
        }
    }
}

void rf::VoiceSet::StopIfIsStinger(long long stopTime, long long playhead)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetStingerHandle())
        {
            m_voices[i].Stop(stopTime, playhead);
        }
    }
}

void rf::VoiceSet::Fade(SoundEffectHandle soundEffectHandle,
                        long long startTime,
                        float amplitude,
                        int sampleDuration,
                        long long playhead,
                        bool stopOnDone)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetSoundEffectHandle() == soundEffectHandle)
        {
            m_voices[i].Fade(startTime, amplitude, sampleDuration, playhead, stopOnDone);
        }
    }
}

void rf::VoiceSet::SetAmplitudeBySoundEffectHandle(SoundEffectHandle soundEffectHandle, float amplitude)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetSoundEffectHandle() == soundEffectHandle)
        {
            m_voices[i].SetAmplitude(amplitude);
        }
    }
}

void rf::VoiceSet::SetPitchBySoundEffectHandle(SoundEffectHandle soundEffectHandle, float pitch)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetSoundEffectHandle() == soundEffectHandle)
        {
            m_voices[i].SetPitch(pitch);
        }
    }
}

void rf::VoiceSet::SetPositionBySoundEffectHandle(SoundEffectHandle soundEffectHandle,
                                                  const PositioningParameters& positioningParameters,
                                                  bool interpolate)
{
    for (int i = 0; i < m_numVoices; ++i)
    {
        if (m_voices[i].GetSoundEffectHandle() == soundEffectHandle)
        {
            m_voices[i].SetPosition(positioningParameters, interpolate);
        }
    }
}

int rf::VoiceSet::GetNumVoices() const
{
    return m_numVoices;
}