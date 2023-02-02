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
#include <stdint.h>

#include "identifiers.h"

namespace rf
{
struct TransitionCondition
{
    using Flags = uint32_t;
    struct Flag
    {
        enum : Flags
        {
            None = 0u,
            OddBars = 1u,
            EvenBars = 2u,
            LessThanBars = 4u,
            LessThanEqualBars = 8u,
            GreaterThanBars = 16u,
            GreaterThanEqualBars = 32u,
            EqualBars = 64u,
            LessThanBeats = 128u,
            LessThanEqualBeats = 256u,
            GreaterThanBeats = 512u,
            GreaterThanEqualBeats = 1024u,
            EqualBeats = 2048u,
        };
        Flag() = delete;
    };

    static constexpr int k_dataSize = 16;
    uint8_t m_userData[k_dataSize] = {};
    CueHandle m_cueHandle;
    int m_bar = -1;
    int m_beat = -1;
    Flags m_flags = Flag::None;

    TransitionCondition();
    bool Matches(const void* userData, CueHandle currentCueHandle, int currentBar, int currentBeat) const;
};
}  // namespace rf