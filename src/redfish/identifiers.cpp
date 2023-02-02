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

#include "identifiers.h"

#define RF_HANDLE_IMPLEMENTATION(name)                     \
    unsigned int rf::name::s_currentValue = rf::InvalidId; \
    bool rf::name::operator==(const name& other) const     \
    {                                                      \
        return m_id == other.m_id;                         \
    }                                                      \
    bool rf::name::operator!=(const name& other) const     \
    {                                                      \
        return m_id != other.m_id;                         \
    }                                                      \
    rf::name::operator bool() const                        \
    {                                                      \
        return m_id > InvalidId;                           \
    }                                                      \
    rf::name rf::Create##name()                            \
    {                                                      \
        ++name::s_currentValue;                            \
        name handle;                                       \
        handle.m_id = name::s_currentValue;                \
        return handle;                                     \
    }

RF_HANDLE_IMPLEMENTATION(AudioHandle);
RF_HANDLE_IMPLEMENTATION(SoundEffectHandle);
RF_HANDLE_IMPLEMENTATION(MixGroupHandle);
RF_HANDLE_IMPLEMENTATION(CueHandle);
RF_HANDLE_IMPLEMENTATION(TransitionHandle);
RF_HANDLE_IMPLEMENTATION(StingerHandle);

#undef RF_HANDLE_IMPLEMENTATION
