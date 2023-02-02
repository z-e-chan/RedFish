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
#include "buffer.h"
#include "dspbase.h"

namespace rf
{
class DelayDSP : public DSPBase
{
public:
    DelayDSP(const AudioSpec& spec);
    DelayDSP(const DelayDSP&) = delete;
    DelayDSP(DelayDSP&&) = delete;
    DelayDSP& operator=(const DelayDSP&) = delete;
    DelayDSP& operator=(DelayDSP&&) = delete;
    ~DelayDSP();

    void SetDelay(int delay);
    void SetFeedback(float feedback);
    void Process(MixItem* mixItem, int bufferSize) override final;

private:
    Buffer* m_buffer = nullptr;
    Buffer* m_feedbackBuffer = nullptr;
    int m_delay = 0;
    float m_feedback = 0;
    int m_delayBufferSize = 0;
    int m_readHead = 0;
};
}  // namespace rf