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
class Buffer;

struct MixItem
{
    MixGroupHandle m_mixGroupHandle;
    int m_channels = 0;
    int m_bufferSize = 0;
    Buffer* m_arrayOfChannels = nullptr;

    MixItem(int channels, int bufferSize);
    MixItem(const MixItem& mixItem);
    MixItem(MixItem&& mixItem);
    MixItem& operator=(const MixItem& mixItem);
    MixItem& operator=(MixItem&& mixItem);
    ~MixItem();

    void Sum(const MixItem& item, float amplitude = 1.0f);
    void Multiply(const MixItem& item);
    void ZeroOut();
    void Set(float value);
    void ToInterleavedBuffer(float* buffer);
    float GetPeakAmplitude() const;
    float GetPeakAmplitudeForChannel(int channel) const;

private:
    void Allocate(int channels, int bufferSize);
    void Free();
};
}  // namespace rf