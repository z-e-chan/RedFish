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

namespace rf
{
class AudioTimeline;

typedef void (*AudioCommandCallback)(AudioTimeline* timeline, void* command);
static constexpr int k_audioCommandSize = 80;

struct AudioCommand
{
    AudioCommandCallback m_callback = nullptr;
    uint8_t m_data[k_audioCommandSize];
};

template <class T, typename... Args>
T& EncodeAudioCommand(AudioCommand* cmd, Args&&... args)
{
    cmd->m_callback = T::s_callback;
    static_assert(sizeof(T) <= k_audioCommandSize, "Audio Command is larger than 80 bytes. Increase k_audioCommandSize.");
    return *new (cmd->m_data) T(std::forward<Args>(args)...);
}
}  // namespace rf