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
static constexpr unsigned int InvalidId = 0;

#define RF_HANDLE(name)                           \
    struct name                                   \
    {                                             \
        unsigned int m_id = InvalidId;            \
        static unsigned int s_currentValue;       \
        bool operator==(const name& other) const; \
        bool operator!=(const name& other) const; \
        explicit operator bool() const;           \
    };                                            \
    name Create##name();

RF_HANDLE(AudioHandle);
RF_HANDLE(SoundEffectHandle);
RF_HANDLE(MixGroupHandle);
RF_HANDLE(CueHandle);
RF_HANDLE(TransitionHandle);
RF_HANDLE(StingerHandle);

#undef RF_HANDLE
}  // namespace rf