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

#include "buffer.h"

#include "allocator.h"
#include "assert.h"

#define RF_SIMD_LOOP(instruction)                                               \
    for (int i = 0; i < m_numSimdIterations; ++i)                               \
    {                                                                           \
        *(m_buffer + i) = instruction(*(m_buffer + i), *(buffer.m_buffer + i)); \
    }

rf::Buffer::Buffer(int size)
{
    Allocate(size);
}

rf::Buffer::Buffer(const Buffer& buffer)
{
    Allocate(buffer.m_size);
    memcpy(m_buffer, buffer.m_buffer, m_bytes);
}

rf::Buffer::Buffer(Buffer&& buffer)
{
    m_size = buffer.m_size;
    m_bytes = buffer.m_bytes;
    m_numSimdIterations = buffer.m_numSimdIterations;
    m_buffer = buffer.m_buffer;

    buffer.m_size = 0;
    buffer.m_bytes = 0;
    buffer.m_numSimdIterations = 0;
    buffer.m_buffer = nullptr;
}

rf::Buffer& rf::Buffer::operator=(const Buffer& buffer)
{
    if (this != &buffer)
    {
        RF_ASSERT(m_size == buffer.m_size, "Expected buffer to be same size.");
        RF_ASSERT(m_buffer, "Expected buffer to be valid.");
        memcpy(m_buffer, buffer.m_buffer, m_bytes);
        m_size = buffer.m_size;
        m_bytes = buffer.m_bytes;
        m_numSimdIterations = buffer.m_numSimdIterations;
    }

    return *this;
}

rf::Buffer& rf::Buffer::operator=(Buffer&& buffer)
{
    if (this != &buffer)
    {
        m_size = buffer.m_size;
        m_bytes = buffer.m_bytes;
        m_numSimdIterations = buffer.m_numSimdIterations;
        m_buffer = buffer.m_buffer;

        buffer.m_size = 0;
        buffer.m_bytes = 0;
        buffer.m_numSimdIterations = 0;
        buffer.m_buffer = nullptr;
    }

    return *this;
}

float& rf::Buffer::operator[](int index)
{
#if RF_USE_SSE || RF_USE_AVX || RF_USE_AVX_512
    return reinterpret_cast<float*>(m_buffer)[index];
#else
    return m_buffer[index];
#endif
}

const float& rf::Buffer::operator[](int index) const
{
#if RF_USE_SSE || RF_USE_AVX || RF_USE_AVX_512
    return reinterpret_cast<float*>(m_buffer)[index];
#else
    return m_buffer[index];
#endif
}

rf::Buffer::~Buffer()
{
    Free();
}

void rf::Buffer::ZeroOut()
{
    memset(m_buffer, 0, m_bytes);
}

void rf::Buffer::Set(float value)
{
    float* buffer = GetAsFloatBuffer();
    for (int i = 0; i < m_size; ++i)
    {
        buffer[i] = value;
    }
}

void rf::Buffer::Multiply(const Buffer& buffer)
{
    RF_ASSERT(m_size == buffer.m_size, "Buffers must be the same size");

#if RF_USE_SSE
    RF_SIMD_LOOP(_mm_mul_ps);
#elif RF_USE_AVX
    RF_SIMD_LOOP(_mm256_mul_ps);
#elif RF_USE_AVX_512
    RF_SIMD_LOOP(_mm512_mul_ps);
#else
    for (int i = 0; i < m_size; ++i)
    {
        m_buffer[i] = m_buffer[i] * buffer[i];
    }
#endif
}

void rf::Buffer::ScalarMultiply(float scalar)
{
#if RF_USE_SSE
    const __m128 simdScalar = _mm_set1_ps(scalar);
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        *(m_buffer + i) = _mm_mul_ps(*(m_buffer + i), simdScalar);
    }
#elif RF_USE_AVX
    const __m256 simdScalar = _mm256_set1_ps(scalar);
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        *(m_buffer + i) = _mm256_mul_ps(*(m_buffer + i), simdScalar);
    }
#elif RF_USE_AVX_512
    const __m512 simdScalar = _mm512_set1_ps(scalar);
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        *(m_buffer + i) = _mm512_mul_ps(*(m_buffer + i), simdScalar);
    }
#else
    for (int i = 0; i < m_size; ++i)
    {
        m_buffer[i] = m_buffer[i] * scalar;
    }
#endif
}

void rf::Buffer::Sum(const Buffer& buffer, float amplitude)
{
    RF_ASSERT(m_size == buffer.m_size, "Buffers must be the same size");

#if RF_USE_SSE
    const __m128 simdScalar = _mm_set1_ps(amplitude);
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        *(m_buffer + i) = _mm_fmadd_ps(*(buffer.m_buffer + i), simdScalar, *(m_buffer + i));
    }
#elif RF_USE_AVX
    const __m256 simdScalar = _mm256_set1_ps(amplitude);
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        *(m_buffer + i) = _mm256_fmadd_ps(*(buffer.m_buffer + i), simdScalar, *(m_buffer + i));
    }
#elif RF_USE_AVX_512
    const __m512 simdScalar = _mm512_set1_ps(amplitude);
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        *(m_buffer + i) = _mm512_fmadd_ps(*(buffer.m_buffer + i), simdScalar, *(m_buffer + i));
    }
#else
    for (int i = 0; i < m_size; ++i)
    {
        m_buffer[i] = m_buffer[i] + (buffer[i] * amplitude);
    }
#endif
}

void rf::Buffer::Subtract(const Buffer& buffer)
{
    RF_ASSERT(m_size == buffer.m_size, "Buffers must be the same size");

#if RF_USE_SSE
    RF_SIMD_LOOP(_mm_sub_ps);
#elif RF_USE_AVX
    RF_SIMD_LOOP(_mm256_sub_ps);
#elif RF_USE_AVX_512
    RF_SIMD_LOOP(_mm512_sub_ps);
#else
    for (int i = 0; i < m_size; ++i)
    {
        m_buffer[i] = m_buffer[i] - buffer[i];
    }
#endif
}

float* rf::Buffer::GetAsFloatBuffer()
{
    return reinterpret_cast<float*>(m_buffer);
}

const float* rf::Buffer::GetAsFloatBuffer() const
{
    return reinterpret_cast<float*>(m_buffer);
}

float rf::Buffer::GetMax() const
{
#if RF_USE_SSE
    __m128 maxValue = _mm_setzero_ps();
    __m128 negMaxValue = _mm_setzero_ps();
    __m128i signMask = _mm_set1_epi32(0x80000000);
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        __m128 value = m_buffer[i];
        __m128 absValue = _mm_andnot_ps(*(__m128*)&signMask, value);
        __m128 cmpMask = _mm_cmpgt_ps(absValue, maxValue);
        maxValue = _mm_or_ps(_mm_and_ps(cmpMask, absValue), _mm_andnot_ps(cmpMask, maxValue));
        cmpMask = _mm_cmpgt_ps(value, negMaxValue);
        negMaxValue = _mm_or_ps(_mm_and_ps(cmpMask, value), _mm_andnot_ps(cmpMask, negMaxValue));
    }
    maxValue = _mm_max_ps(maxValue, negMaxValue);
    __m128 permValue = _mm_permute_ps(maxValue, _MM_SHUFFLE(1, 0, 3, 2));
    maxValue = _mm_max_ps(maxValue, permValue);
    permValue = _mm_permute_ps(maxValue, _MM_SHUFFLE(2, 3, 0, 1));
    maxValue = _mm_max_ps(maxValue, permValue);
    return _mm_cvtss_f32(maxValue);
#else
    const float* buffer = GetAsFloatBuffer();
    float max = 0.0f;
    for (int i = 0; i < m_size; ++i)
    {
        if (buffer[i] > max)
        {
            max = buffer[i];
        }
    }
    return max;
#endif
}

float rf::Buffer::GetAbsoluteMax() const
{
#if RF_USE_SSE
    __m128 maxValue = _mm_setzero_ps();
    for (int i = 0; i < m_numSimdIterations; ++i)
    {
        __m128 value = m_buffer[i];
        __m128 absValue = _mm_andnot_ps(_mm_set1_ps(-0.0f), value);
        __m128 cmpMask = _mm_cmpgt_ps(absValue, maxValue);
        maxValue = _mm_or_ps(_mm_and_ps(cmpMask, absValue), _mm_andnot_ps(cmpMask, maxValue));
    }
    __m128 permValue = _mm_permute_ps(maxValue, _MM_SHUFFLE(1, 0, 3, 2));
    maxValue = _mm_max_ps(maxValue, permValue);
    permValue = _mm_permute_ps(maxValue, _MM_SHUFFLE(2, 3, 0, 1));
    maxValue = _mm_max_ps(maxValue, permValue);
    return _mm_cvtss_f32(maxValue);
#else
    const float* buffer = GetAsFloatBuffer();
    float max = 0.0f;
    for (int i = 0; i < m_size; ++i)
    {
        const float value = buffer[i];
        const float absValue = value < 0.0f ? -value : value;
        if (absValue > max)
        {
            max = absValue;
        }
    }
    return max;
#endif
}

void rf::Buffer::Allocate(int size)
{
    Free();

    m_size = size;
    m_bytes = sizeof(float) * size;
    int sizeOfType = sizeof(float);

#if RF_USE_SSE
    sizeOfType = sizeof(__m128);
    m_buffer = static_cast<__m128*>(Allocator::s_allocate(m_bytes, "Buffer", alignof(__m128)));
#elif RF_USE_AVX
    sizeOfType = sizeof(__m256);
    m_buffer = static_cast<__m256*>(Allocator::s_allocate(m_bytes, "Buffer", alignof(__m256)));
#elif RF_USE_AVX_512
    sizeOfType = sizeof(__m512);
    m_buffer = static_cast<__m512*>(Allocator::s_allocate(m_bytes, "Buffer", alignof(__m512)));
#else
    m_buffer = static_cast<float*>(Allocator::s_allocate(m_bytes, "Buffer", alignof(float)));
#endif

    m_numSimdIterations = m_bytes / sizeOfType;
    ZeroOut();
}

void rf::Buffer::Free()
{
    if (m_buffer)
    {
        Allocator::s_deallocate(m_buffer);
        m_buffer = nullptr;
        m_size = 0;
        m_bytes = 0;
        m_numSimdIterations = 0;
    }
}

#undef RF_SIMD_LOOP