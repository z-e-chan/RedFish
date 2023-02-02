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

#include "positioningdsp.h"

#include <algorithm>

#include "functions.h"

rf::PositioningDSP::PositioningDSP(const AudioSpec& spec)
    : DSPBase(spec)
    , m_gain(spec)
    , m_hpf(spec)
    , m_lpf(spec)
    , m_pan(spec)
{
    m_hpf.SetOrder(2);
    m_lpf.SetOrder(2);
}

void rf::PositioningDSP::SetPositioningParameters(const PositioningParameters& parameters, bool interpolate)
{
    m_parameters = parameters;

    const float distanceCurvePercent = CalculateDistanceCurvePercent(m_parameters);
    const float percentDifference = 1.0f - distanceCurvePercent;
    const float amp = GetAttenuatedAmplitude(m_parameters);

    const float hpfCutoff = (m_parameters.m_maxHpfCutoff * distanceCurvePercent) + (PluginUtils::k_minFilterCutoff * percentDifference);
    const float lpfCutoff = (m_parameters.m_maxLpfCutoff * distanceCurvePercent) + (PluginUtils::k_maxFilterCutoff * percentDifference);

    if (!interpolate)
    {
        // If we are not interpolating, then we should reset the filters before setting the cutoff.
        // No interpolation means that we a 'playing a new sound' rather than updated a currently playing sound.
        // Therefore, we do not want to bring old data from the filter's delay lines into the new sound.
        // If we do that, we get a click. So we reset the filters.
        m_hpf.ResetDelayLines();
        m_lpf.ResetDelayLines();
    }

    m_pan.SetAngle(m_parameters.m_panAngle, interpolate);
    m_gain.SetAmplitude(amp, interpolate);
    m_hpf.SetCutoff(hpfCutoff);
    m_lpf.SetCutoff(lpfCutoff);
}

void rf::PositioningDSP::SetPositioningParameters(const PositioningParameters& parameters)
{
    SetPositioningParameters(parameters, true);
}

void rf::PositioningDSP::Process(MixItem* mixItem, int bufferSize)
{
    if (m_bypass || !m_parameters.m_enable)
    {
        return;
    }

    m_gain.Process(mixItem, bufferSize);
    m_hpf.Process(mixItem, bufferSize);
    m_lpf.Process(mixItem, bufferSize);
    m_pan.Process(mixItem, bufferSize);
}

float rf::PositioningDSP::GetAttenuatedAmplitude(const PositioningParameters& parameters)
{
    const float distanceCurvePercent = CalculateDistanceCurvePercent(parameters);
    const float percentDifference = 1.0f - distanceCurvePercent;

    const float maxAttenuationAmp = Functions::DecibelToAmplitude(parameters.m_maxAttenuationDb);
    const float amp = std::max(percentDifference, maxAttenuationAmp);
    return amp;
}

float rf::PositioningDSP::CalculateDistanceCurvePercent(const PositioningParameters& parameters)
{
    const float denomenator = parameters.m_maxDistance - parameters.m_minDistance;
    if (Functions::FloatEquality(denomenator, 0.0f))
    {
        return 1.0f;
    }

    const float linearPercent =
        (Functions::Clamp(parameters.m_currentDistance, parameters.m_minDistance, parameters.m_maxDistance) - parameters.m_minDistance) / denomenator;

    switch (parameters.m_distanceCurveType)
    {
        case PositioningParameters::DistanceCurve::Linear: return linearPercent;
        case PositioningParameters::DistanceCurve::EqualPower: return sqrtf(linearPercent);
        case PositioningParameters::DistanceCurve::Quadratic: return linearPercent * linearPercent;
        default: return linearPercent;
    }
}