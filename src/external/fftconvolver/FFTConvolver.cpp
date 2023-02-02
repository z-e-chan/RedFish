// ==================================================================================
// Copyright (c) 2017 HiFi-LoFi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ==================================================================================

#include "FFTConvolver.h"

#include <cassert>
#include <cmath>

#if defined(FFTCONVOLVER_USE_SSE)
#    include <xmmintrin.h>
#endif

namespace fftconvolver
{
FFTConvolver::FFTConvolver()
    : _blockSize(0)
    , _segSize(0)
    , _segCount(0)
    , _fftComplexSize(0)
    , _segments()
    , _segmentsIR()
    , _master_ir_segments()
    , _fftBuffer()
    , _fft()
    , _preMultiplied()
    , _conv()
    , _overlap()
    , _current(0)
    , _inputBuffer()
    , _inputBufferFill(0)
{
}

FFTConvolver::~FFTConvolver()
{
    reset();
}

void FFTConvolver::reset()
{
    for (size_t i = 0; i < _segCount; ++i)
    {
        delete _segments[i];

        /* RedFish Modification */
        for (int j = 0; j < FFTCONVOLER_MAX_NUM_IR; ++j)
        {
            delete _segmentsIR[j][i];
        }
    }

    _blockSize = 0;
    _segSize = 0;
    _segCount = 0;
    _fftComplexSize = 0;
    _segments.clear();

    /* RedFish Modification */
    for (int i = 0; i < FFTCONVOLER_MAX_NUM_IR; ++i)
    {
        _segmentsIR[i].clear();
    }
    _master_ir_segments.clear();

    _fftBuffer.clear();
    _fft.init(0);
    _preMultiplied.clear();
    _conv.clear();
    _overlap.clear();
    _current = 0;
    _inputBuffer.clear();
    _inputBufferFill = 0;
}

/* RedFish Modification */
void FFTConvolver::setImpulseResponseAmplitudes(const float amplitudes[FFTCONVOLER_MAX_NUM_IR])
{
    for (int i = 0; i < _segCount; ++i)
    {
        const fftconvolver::Sample *s1_re = _segmentsIR[0][i]->re();
        const fftconvolver::Sample *s1_im = _segmentsIR[0][i]->im();
        const fftconvolver::Sample *s2_re = _segmentsIR[1][i]->re();
        const fftconvolver::Sample *s2_im = _segmentsIR[1][i]->im();
        const fftconvolver::Sample *s3_re = _segmentsIR[2][i]->re();
        const fftconvolver::Sample *s3_im = _segmentsIR[2][i]->im();
        fftconvolver::Sample *__restrict master_re = _master_ir_segments[i]->re();
        fftconvolver::Sample *__restrict master_im = _master_ir_segments[i]->im();

        for (int j = 0; j < _fftComplexSize; ++j)
        {
            float temp_re = s1_re[j] * amplitudes[0];
            temp_re += s2_re[j] * amplitudes[1];
            temp_re += s3_re[j] * amplitudes[2];
            float temp_im = s1_im[j] * amplitudes[0];
            temp_im += s2_im[j] * amplitudes[1];
            temp_im += s3_im[j] * amplitudes[2];
            master_re[j] = temp_re;
            master_im[j] = temp_im;
        }
    }
}

/* RedFish Modification */
bool FFTConvolver::init(size_t blockSize, Sample **ir, size_t irLen)
{
    reset();

    if (blockSize == 0)
    {
        return false;
    }

    /* RedFish Modification */
    // Ignore zeros at the end of the impulse response because they only waste computation time
    // while (irLen > 0 && ::fabs(ir[0][irLen-1]) < 0.000001f)
    //{
    //  --irLen;
    //}

    if (irLen == 0)
    {
        return true;
    }

    _blockSize = NextPowerOf2(blockSize);
    _segSize = 2 * _blockSize;
    _segCount = static_cast<size_t>(::ceil(static_cast<float>(irLen) / static_cast<float>(_blockSize)));
    _fftComplexSize = audiofft::AudioFFT::ComplexSize(_segSize);

    // FFT
    _fft.init(_segSize);
    _fftBuffer.resize(_segSize);

    // Prepare segments
    for (size_t i = 0; i < _segCount; ++i)
    {
        _segments.push_back(new SplitComplex(_fftComplexSize));
    }

    /* RedFish Modification */
    for (int j = 0; j < FFTCONVOLER_MAX_NUM_IR; ++j)
    {
        // Prepare IR
        for (size_t i = 0; i < _segCount; ++i)
        {
            /* RedFish Modification */
            SplitComplex *master_segment = new SplitComplex(_fftComplexSize);
            master_segment->setZero();
            _master_ir_segments.push_back(master_segment);
            SplitComplex *segment = new SplitComplex(_fftComplexSize);
            const size_t remaining = irLen - (i * _blockSize);
            const size_t sizeCopy = (remaining >= _blockSize) ? _blockSize : remaining;
            /* RedFish Modification */
            CopyAndPad(_fftBuffer, &ir[j][i * _blockSize], sizeCopy);
            _fft.fft(_fftBuffer.data(), segment->re(), segment->im());
            _segmentsIR[j].push_back(segment);
        }
    }

    /* RedFish Modification */
    for (int i = 0; i < _segCount; ++i)
    {
        for (int j = 0; j < _fftComplexSize; ++j)
        {
            _master_ir_segments[i]->re()[j] = _segmentsIR[0][i]->re()[j];
            _master_ir_segments[i]->im()[j] = _segmentsIR[0][i]->im()[j];
        }
    }

    // Prepare convolution buffers
    _preMultiplied.resize(_fftComplexSize);
    _conv.resize(_fftComplexSize);
    _overlap.resize(_blockSize);

    // Prepare input buffer
    _inputBuffer.resize(_blockSize);
    _inputBufferFill = 0;

    // Reset current position
    _current = 0;

    return true;
}

void FFTConvolver::process(const Sample *input, Sample *output, size_t len)
{
    if (_segCount == 0)
    {
        ::memset(output, 0, len * sizeof(Sample));
        return;
    }

    size_t processed = 0;
    while (processed < len)
    {
        const bool inputBufferWasEmpty = (_inputBufferFill == 0);
        const size_t processing = std::min(len - processed, _blockSize - _inputBufferFill);
        const size_t inputBufferPos = _inputBufferFill;
        ::memcpy(_inputBuffer.data() + inputBufferPos, input + processed, processing * sizeof(Sample));

        // Forward FFT
        CopyAndPad(_fftBuffer, &_inputBuffer[0], _blockSize);
        _fft.fft(_fftBuffer.data(), _segments[_current]->re(), _segments[_current]->im());

        // Complex multiplication
        if (inputBufferWasEmpty)
        {
            _preMultiplied.setZero();
            for (size_t i = 1; i < _segCount; ++i)
            {
                const size_t indexIr = i;
                const size_t indexAudio = (_current + i) % _segCount;

                /* RedFish Modification */
                ComplexMultiplyAccumulate(_preMultiplied, *_master_ir_segments[indexIr], *_segments[indexAudio]);
            }
        }

        _conv.copyFrom(_preMultiplied);
        /* RedFish Modification */
        ComplexMultiplyAccumulate(_conv, *_segments[_current], *_master_ir_segments[0]);

        // Backward FFT
        _fft.ifft(_fftBuffer.data(), _conv.re(), _conv.im());

        // Add overlap
        Sum(output + processed, _fftBuffer.data() + inputBufferPos, _overlap.data() + inputBufferPos, processing);

        // Input buffer full => Next block
        _inputBufferFill += processing;
        if (_inputBufferFill == _blockSize)
        {
            // Input buffer is empty again now
            _inputBuffer.setZero();
            _inputBufferFill = 0;

            // Save the overlap
            ::memcpy(_overlap.data(), _fftBuffer.data() + _blockSize, _blockSize * sizeof(Sample));

            // Update current segment
            _current = (_current > 0) ? (_current - 1) : (_segCount - 1);
        }

        processed += processing;
    }
}

}  // End of namespace fftconvolver
