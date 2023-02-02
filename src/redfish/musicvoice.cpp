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

#include "musicvoice.h"

#include "assert.h"
#include "audiospec.h"
#include "functions.h"
#include "musictransitionrequest.h"

rf::MusicVoice::MusicVoice(const AudioSpec& spec)
    : BaseVoice(spec.m_bufferSize)
{
}

void rf::MusicVoice::Play(long long startTime,
                          const MusicDatabase::CueData& cueData,
                          int playCount,
                          int layerIndex,
                          const AudioData** audioData,
                          float amplitude)
{
    RF_ASSERT(layerIndex >= 0 && layerIndex < cueData.m_numLayers, "layerIndex out of bounds");
    const Layer& layer = cueData.m_layers[layerIndex];
    const float cueAmplitude = Functions::DecibelToAmplitude(cueData.m_gainDb);
    const float layerAmplitude = Functions::DecibelToAmplitude(layer.m_gainDb);
    const float finalAmplitude = cueAmplitude * layerAmplitude * amplitude;
    BaseVoice::PlayParams params;
    params.m_audioData = audioData[layer.m_audioDataIndex];
    params.m_audioHandle = layer.m_audioHandle;
    params.m_mixGroupHandle = layer.m_mixGroupHandle;
    params.m_startTime = startTime;
    params.m_playCount = playCount;
    params.m_pitch = 1.0f;
    params.m_amplitude = finalAmplitude;
    PlayBase(params);
}

void rf::MusicVoice::Play(long long startTime, const MusicDatabase::CueData& cueData, int playCount, int layerIndex, const AudioData** audioData)
{
    Play(startTime, cueData, playCount, layerIndex, audioData, 1.0f);
}