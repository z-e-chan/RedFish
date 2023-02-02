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
#include "defines.h"

#if RF_USE_SSE
#    include <emmintrin.h>
#    include <immintrin.h>
#    include <xmmintrin.h>
#elif RF_USE_AVX
#    include <immintrin.h>
#elif RF_USE_AVX_512
#    include <immintrin.h>
#endif

namespace rf
{
class Buffer
{
public:
    Buffer(int size);
    Buffer(const Buffer& buffer);
    Buffer(Buffer&& buffer);
    Buffer& operator=(const Buffer& buffer);
    Buffer& operator=(Buffer&& buffer);
    float& operator[](int index);
    const float& operator[](int index) const;
    ~Buffer();

    void ZeroOut();
    void Set(float value);
    void Multiply(const Buffer& buffer);
    void ScalarMultiply(float scalar);
    void Sum(const Buffer& buffer, float amplitude);
    void Subtract(const Buffer& buffer);
    float* GetAsFloatBuffer();
    const float* GetAsFloatBuffer() const;
    float GetMax() const;
    float GetAbsoluteMax() const;

    int m_size = 0;
    int m_bytes = 0;

private:
    int m_numSimdIterations = 0;

#if RF_USE_SSE
    __m128* m_buffer = nullptr;
#elif RF_USE_AVX
    __m256* m_buffer = nullptr;
#elif RF_USE_AVX_512
    __m512* m_buffer = nullptr;
#else
    float* m_buffer = nullptr;
#endif

    void Allocate(int size);
    void Free();
};
}  // namespace rf