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
#include "defines.h"
#include "layer.h"
#include "meter.h"

namespace rf
{
class AssetSystem;
class MixGroup;
struct CueParameters;

class Cue
{
public:
    Cue(const CueParameters& cueParameters, const AssetSystem* assetSystem, int index);
    Cue(const Cue&) = delete;
    Cue(Cue&&) = delete;
    Cue& operator=(const Cue&) = delete;
    Cue& operator=(Cue&&) = delete;
    ~Cue() = default;

    explicit operator bool() const;

    const Layer& GetLayer(int index) const;
    int GetNumLayers() const;
    CueHandle GetCueHandle() const;
    const Meter& GetMeter() const;
    float GetTempo() const;
    float GetGainDb() const;
    const char* GetName() const;

private:
    char m_name[RF_MAX_NAME_SIZE];
    Layer m_layers[RF_MAX_CUE_LAYERS];
    CueHandle m_cueHandle;
    Meter m_meter;
    float m_tempo = 0.0f;
    float m_gainDb = 0.0f;
    int m_numLayers = 0;
    int m_index = -1;
};

struct CueParameters
{
    Layer m_layers[RF_MAX_CUE_LAYERS] = {};
    Meter m_meter;
    const char* m_name = nullptr;
    float m_tempo = 0.0f;
    float m_gainDb = 0.0f;
    int m_numLayers = 0;

    void AddLayer(AudioHandle audioHandle, const MixGroup* mixGroup, float volumeDb);
    void AddLayer(AudioHandle audioHandle, const MixGroup* mixGroup);
    void ClearLayers();
};
}  // namespace rf