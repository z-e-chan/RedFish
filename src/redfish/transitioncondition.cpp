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

#include "transitioncondition.h"

#include <cstring>

#include "assert.h"

rf::TransitionCondition::TransitionCondition()
{
#if RF_ENABLE_ASSERTS
    for (int i = 0; i < k_dataSize; ++i)
    {
        RF_ASSERT(m_userData[i] == 0, "Expected data to be zeroed");
    }
#endif
}

bool rf::TransitionCondition::Matches(const void* userData, CueHandle currentCueHandle, int currentBar, int currentBeat) const
{
    // Compare m_cueHandle with currentCueHandle
    // If !m_cueHandle, we pass the comparison trivially.
    {
        if (m_cueHandle)
        {
            if (m_cueHandle != currentCueHandle)
            {
                return false;
            }
        }
    }

    // Compare m_userData with userData
    const int compareValue = memcmp(userData, m_userData, k_dataSize);
    if (compareValue != 0)
    {
        // userData does not match
        // Fail comparison.
        return false;
    }

    // Test Bars
    {
        const bool evenBar = currentBar % 2 == 0;

        if (m_flags & Flag::OddBars)
        {
            if (currentBar <= 0)
            {
                return false;
            }

            if (evenBar)
            {
                return false;
            }
        }

        if (m_flags & Flag::EvenBars)
        {
            if (currentBar <= 0)
            {
                return false;
            }

            if (!evenBar)
            {
                return false;
            }
        }

        if (m_flags & Flag::LessThanBars)
        {
            if (currentBar <= 0)
            {
                return false;
            }

            const bool result = currentBar < m_bar;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::LessThanEqualBars)
        {
            if (currentBar <= 0)
            {
                return false;
            }

            const bool result = currentBar <= m_bar;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::GreaterThanBars)
        {
            if (currentBar <= 0)
            {
                return false;
            }

            const bool result = currentBar > m_bar;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::GreaterThanEqualBars)
        {
            if (currentBar <= 0)
            {
                return false;
            }

            const bool result = currentBar >= m_bar;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::EqualBars)
        {
            if (currentBar <= 0)
            {
                return false;
            }

            const bool result = currentBar == m_bar;
            if (!result)
            {
                return false;
            }
        }
    }

    // Test Beats
    {
        if (m_flags & Flag::LessThanBeats)
        {
            if (currentBeat <= 0)
            {
                return false;
            }

            const bool result = currentBeat < m_beat;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::LessThanEqualBeats)
        {
            if (currentBeat <= 0)
            {
                return false;
            }

            const bool result = currentBeat <= m_beat;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::GreaterThanBeats)
        {
            if (currentBeat <= 0)
            {
                return false;
            }

            const bool result = currentBeat > m_beat;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::GreaterThanEqualBeats)
        {
            if (currentBeat <= 0)
            {
                return false;
            }

            const bool result = currentBeat >= m_beat;
            if (!result)
            {
                return false;
            }
        }

        if (m_flags & Flag::EqualBeats)
        {
            if (currentBeat <= 0)
            {
                return false;
            }

            const bool result = currentBeat == m_beat;
            if (!result)
            {
                return false;
            }
        }
    }

    return true;
}