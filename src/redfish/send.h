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
#include "identifiers.h"

namespace rf
{
class CommandProcessor;

class Send
{
public:
    Send(CommandProcessor* commands, int sendIndex, MixGroupHandle sendToMixGroupHandle);
    Send(const Send&) = delete;
    Send(Send&&) = delete;
    Send& operator=(const Send&) = delete;
    Send& operator=(Send&&) = delete;
    explicit operator bool() const;
    ~Send() = default;

    void SetVolumeDb(float volumeDb);
    float GetVolumeDb() const;
    MixGroupHandle GetSendToMixGroupHandle() const;

private:
    CommandProcessor* m_commands = nullptr;
    float m_amplitude = 1.0f;
    int m_sendIndex = -1;
    MixGroupHandle m_sendToMixGroupHandle;
};
}  // namespace rf