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

#include "cue.h"

#include "assert.h"
#include "assetsystem.h"
#include "mixgroup.h"

rf::Cue::Cue(const CueParameters& cueParameters, const AssetSystem* assetSystem, int index)
    : m_cueHandle(CreateCueHandle())
    , m_meter(cueParameters.m_meter)
    , m_tempo(cueParameters.m_tempo)
    , m_gainDb(cueParameters.m_gainDb)
    , m_numLayers(cueParameters.m_numLayers)
    , m_index(index)
{
    RF_ASSERT(m_numLayers < RF_MAX_CUE_LAYERS, "Too many layers added. Increase RF_MAX_CUE_LAYERS");
    for (int i = 0; i < m_numLayers; ++i)
    {
        m_layers[i] = cueParameters.m_layers[i];
        m_layers[i].m_audioDataIndex = assetSystem->GetAudioDataIndex(m_layers[i].m_audioHandle);
    }

    if (cueParameters.m_name)
    {
        strcpy_s(m_name, cueParameters.m_name);
    }
}

rf::Cue::operator bool() const
{
    return static_cast<bool>(m_numLayers > 0);
}

const rf::Layer& rf::Cue::GetLayer(int index) const
{
    RF_ASSERT(index >= 0 && index < m_numLayers, "Index out of bounds");
    return m_layers[index];
}

int rf::Cue::GetNumLayers() const
{
    return m_numLayers;
}

rf::CueHandle rf::Cue::GetCueHandle() const
{
    return m_cueHandle;
}

const rf::Meter& rf::Cue::GetMeter() const
{
    return m_meter;
}

float rf::Cue::GetTempo() const
{
    return m_tempo;
}

float rf::Cue::GetGainDb() const
{
    return m_gainDb;
}

const char* rf::Cue::GetName() const
{
    return m_name;
}

void rf::CueParameters::AddLayer(AudioHandle audioHandle, const MixGroup* mixGroup, float volumeDb)
{
    RF_ASSERT(m_numLayers < RF_MAX_CUE_LAYERS, "Too many layers added. Increase RF_MAX_CUE_LAYERS");
    Layer layer;
    layer.m_audioHandle = audioHandle;
    layer.m_mixGroupHandle = mixGroup->GetMixGroupHandle();
    layer.m_gainDb = volumeDb;
    m_layers[m_numLayers++] = layer;
}

void rf::CueParameters::AddLayer(AudioHandle audioHandle, const MixGroup* mixGroup)
{
    AddLayer(audioHandle, mixGroup, 0.0f);
}

void rf::CueParameters::ClearLayers()
{
    memset(m_layers, 0, sizeof(Layer) * RF_MAX_CUE_LAYERS);
    m_numLayers = 0;
}
