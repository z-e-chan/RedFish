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

#include "positioningparameters.h"

#include "functions.h"

bool rf::PositioningParameters::operator==(const PositioningParameters& parameters) const
{
    if (m_distanceCurveType != parameters.m_distanceCurveType)
    {
        return false;
    }

    if (!Functions::FloatEquality(m_panAngle, parameters.m_panAngle))
    {
        return false;
    }

    if (!Functions::FloatEquality(m_currentDistance, parameters.m_currentDistance))
    {
        return false;
    }

    if (!Functions::FloatEquality(m_minDistance, parameters.m_minDistance))
    {
        return false;
    }

    if (!Functions::FloatEquality(m_maxDistance, parameters.m_maxDistance))
    {
        return false;
    }

    if (!Functions::FloatEquality(m_maxAttenuationDb, parameters.m_maxAttenuationDb))
    {
        return false;
    }

    if (!Functions::FloatEquality(m_maxHpfCutoff, parameters.m_maxHpfCutoff))
    {
        return false;
    }

    if (!Functions::FloatEquality(m_maxLpfCutoff, parameters.m_maxLpfCutoff))
    {
        return false;
    }

    if (m_enable != parameters.m_enable)
    {
        return false;
    }

    return true;
}