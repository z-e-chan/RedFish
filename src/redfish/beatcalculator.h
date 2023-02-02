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
#include "meter.h"
#include "sync.h"

namespace rf
{
struct AudioSpec;

class BeatCalculator
{
public:
    double BeatCalc(float tempo, const Meter& meter, const AudioSpec& spec);
    int BeatSwitch(const Sync& sync) const;
    double PreciseBeatSwitch(const Sync& sync) const;

    struct PreciseSampleValues
    {
        double m_onetwentyeigthSamplesPrecise = 0.0;
        double m_sixtyfourthSamplesPrecise = 0.0;
        double m_thirtysecondSamplesPrecise = 0.0;
        double m_sixteenthSamplesPrecise = 0.0;
        double m_eigthSamplesPrecise = 0.0;
        double m_quarterSamplesPrecise = 0.0;
        double m_halfSamplesPrecise = 0.0;
        double m_wholeSamplesPrecise = 0.0;
        double m_barSamplesPrecise = 0.0;
    };

    struct BeatSampleValues
    {
        int m_onetwentyeigthSamples = 0;
        int m_sixtyfourthSamples = 0;
        int m_thirtysecondSamples = 0;
        int m_sixteenthSamples = 0;
        int m_eigthSamples = 0;
        int m_quarterSamples = 0;
        int m_halfSamples = 0;
        int m_wholeSamples = 0;
        int m_barSamples = 0;

        int m_sixtyfourthDottedSamples;
        int m_thirtysecondDottedSamples;
        int m_sixteenthDottedSamples;
        int m_eigthDottedSamples;
        int m_quarterDottedSamples;
        int m_halfDottedSamples;

        int m_sixtyfourthTripletSamples = 0;
        int m_thirtysecondTripletSamples = 0;
        int m_sixteenthTripletSamples = 0;
        int m_eigthTripletSamples = 0;
        int m_quarterTripletSamples = 0;
        int m_halfTripletSamples = 0;
    };

    PreciseSampleValues m_beatPrecise;
    BeatSampleValues m_beatSamples;
};
}  // namespace rf