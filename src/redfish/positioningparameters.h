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

namespace rf
{
struct PositioningParameters
{
    enum class DistanceCurve : int
    {
        Linear,
        EqualPower,
        Quadratic
    };

    // The type of distance curve used to control attenuation.
    DistanceCurve m_distanceCurveType = DistanceCurve::Linear;
    // The pan angle of the sound. -1.0f is left, 0.0f is center, and 1.0f is right.
    float m_panAngle = 0.0f;
    // The current distance of the sound.
    float m_currentDistance = 0.0f;
    // The min distance used for distance attenuation.
    float m_minDistance = 0.0f;
    // The max distance used for distance attenuation.
    float m_maxDistance = 0.0f;
    // The max amount of volume attenuation applied to the sound in decibels when the sound is at or past the max
    // distance.
    float m_maxAttenuationDb = 0.0f;
    // The max amount of Butterworth low-end frequency attenuation applied to the sound when the sound is at or past
    // the max distance.
    float m_maxHpfCutoff = 20.0f;
    // The max amount of Butterworth high-end frequency attenuation applied to the sound when the sound is at or past
    // the max distance.
    float m_maxLpfCutoff = 20000.0f;
    // Enables positioning.
    bool m_enable = false;

    bool operator==(const PositioningParameters& parameters) const;
};
}  // namespace rf