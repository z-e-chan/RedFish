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

#include "voice.h"

#include "assert.h"
#include "functions.h"
#include "layer.h"
#include "playcommands.h"

rf::Voice::Voice(const AudioSpec& spec)
    : BaseVoice(spec.m_bufferSize)
    , m_fader(spec.m_bufferSize)
    , m_positioning(spec)
{
}

void rf::Voice::Play(const AudioData* audioData, const PlayCommand& command, long long startTime)
{
    m_fader.Reset();
    m_positioning.SetPositioningParameters(command.m_positioningParameters, false);
    m_isStopping = false;
    m_stopOnDoneFade = false;

    BaseVoice::PlayParams params;
    params.m_audioData = audioData;
    params.m_audioHandle = command.m_audioHandle;
    params.m_soundEffectHandle = command.m_soundEffectHandle;
    params.m_mixGroupHandle = command.m_mixGroupHandle;
    params.m_startTime = startTime;
    params.m_playCount = command.m_playCount;
    params.m_pitch = command.m_pitch;
    params.m_amplitude = command.m_amplitude;
    PlayBase(params);
}

void rf::Voice::Play(const AudioData* audioData, const Layer& layer, StingerHandle stingerHandle, long long startTime, float amplitude)
{
    m_fader.Reset();
    m_positioning.SetPositioningParameters(PositioningParameters(), false);
    m_isStopping = false;
    m_stopOnDoneFade = false;

    BaseVoice::PlayParams params;
    params.m_audioData = audioData;
    params.m_audioHandle = layer.m_audioHandle;
    params.m_stingerHandle = stingerHandle;
    params.m_mixGroupHandle = layer.m_mixGroupHandle;
    params.m_startTime = startTime;
    params.m_playCount = 1;
    params.m_pitch = 1.0f;
    params.m_amplitude = Functions::DecibelToAmplitude(layer.m_gainDb) * amplitude;
    PlayBase(params);
}

void rf::Voice::Stop(long long stopTime, long long playhead)
{
    m_stopOnDoneFade = false;
    m_isStopping = true;
    m_fader.Update(0.0f, stopTime - playhead, k_stopSamples);
}

void rf::Voice::Fade(long long startTime, float amplitude, int sampleDuration, long long playhead, bool stopOnDone)
{
    m_stopOnDoneFade = stopOnDone;
    m_fader.Update(amplitude, startTime - playhead, sampleDuration);
}

void rf::Voice::SetAmplitude(float amplitude)
{
    m_gain.SetAmplitude(amplitude, true);
}

void rf::Voice::SetPitch(float pitch)
{
    m_pitch = pitch;
}

void rf::Voice::SetPosition(const PositioningParameters& positioningParameters, bool interpolate)
{
    m_positioning.SetPositioningParameters(positioningParameters, interpolate);
}

rf::BaseVoice::Info rf::Voice::FillMixItem(long long playhead, MixItem* outMixItem, int bufferSize, Messenger* messenger)
{
    int startingIndex = 0;
    const bool inFirstWindow = Functions::InFirstWindow(playhead, m_startTime, bufferSize);
    if (inFirstWindow)
    {
        const long long difference = m_startTime - playhead;
        RF_ASSERT(difference <= bufferSize, "Expected this value to be <= bufferSize.");
        startingIndex = static_cast<int>(difference);
    }

    BaseVoice::Info info = FillMixItemBase(playhead, outMixItem, startingIndex, bufferSize - startingIndex, bufferSize, messenger);
    const Result result = UpdateDSP(outMixItem, bufferSize);

    if (info.m_done || result == Result::Stop)
    {
        info.m_stopped = true;
        Reset(messenger);
    }

    return info;
}

void rf::Voice::Reset(Messenger* messenger)
{
    ResetBase(messenger);
    m_fader.Reset();
    m_positioning.SetPositioningParameters(PositioningParameters(), false);
    m_isStopping = false;
    m_stopOnDoneFade = false;
}

rf::Voice::Result rf::Voice::UpdateDSP(MixItem* mixItem, int bufferSize)
{
    const bool isFadingBefore = m_fader.IsFading();
    const bool isFading = m_fader.Process(mixItem, bufferSize);
    const bool isFadingAfter = m_fader.IsFading();

    m_positioning.Process(mixItem, bufferSize);

    const bool fadeIsComplete = !m_isStopping && isFadingBefore && !isFadingAfter;
    const bool stopOnDoneFade = fadeIsComplete && m_stopOnDoneFade;
    const bool stopIsComplete = !isFading && m_isStopping;
    const bool stop = stopIsComplete || stopOnDoneFade;
    if (stop)
    {
        return Result::Stop;
    }

    return Result::None;
}