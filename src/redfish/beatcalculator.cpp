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

#include "beatcalculator.h"

#include <cmath>

#include "assert.h"
#include "audiospec.h"

double rf::BeatCalculator::BeatCalc(float tempo, const Meter& meter, const AudioSpec& spec)
{
    const bool validMeter = meter.m_top > 0 && meter.m_bottom > 0;
    if (tempo <= 0.0f || !validMeter)
    {
        return 0.0;
    }

    // Constants for calculations.
    static constexpr float k_oneTwentyEigth = 0.03125f;
    static constexpr float k_sixtyFourth = 0.0625f;
    static constexpr float k_thirtySecond = 0.125f;
    static constexpr float k_sixteenth = 0.25f;
    static constexpr float k_eigth = 0.5f;
    static constexpr float k_quarter = 1.0f;
    static constexpr float k_half = 2.0f;
    static constexpr float k_whole = 4.0f;

    // Find the number of quarter notes per bar.
    double quarterPerBar = 0.0;
    double scale = 0.0;

    if (meter.m_bottom == 4)
    {
        quarterPerBar = meter.m_top * 1.0;
    }
    else if (meter.m_bottom < 4)
    {
        scale = 4.0 / meter.m_bottom;
        quarterPerBar = meter.m_top * scale;
    }
    else if (meter.m_bottom > 4)
    {
        scale = meter.m_bottom / 4.0;
        quarterPerBar = meter.m_top / scale;
    }

    // Determine how many samples are in a bar by find seconds in a bar and multiplying by sample
    // rate.
    const double ratio = quarterPerBar / tempo;
    const double secondsPerBar = ratio * 60.0;
    const double rawBarSamples = secondsPerBar * spec.m_sampleRate;

    // Constant scale. The above constants are defined for a quarter note beat divsion. We need to
    // scale them for other beat division.
    double constantScale = 0.25 * meter.m_bottom;

    // Calculate the precise number of samples for each beat type that will be the basis for sample
    // math later.
    m_beatPrecise.m_onetwentyeigthSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_oneTwentyEigth);
    m_beatPrecise.m_sixtyfourthSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_sixtyFourth);
    m_beatPrecise.m_thirtysecondSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_thirtySecond);
    m_beatPrecise.m_sixteenthSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_sixteenth);
    m_beatPrecise.m_eigthSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_eigth);
    m_beatPrecise.m_quarterSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_quarter);
    m_beatPrecise.m_halfSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_half);
    m_beatPrecise.m_wholeSamplesPrecise = static_cast<double>(rawBarSamples / meter.m_top * constantScale * k_whole);
    m_beatPrecise.m_barSamplesPrecise = rawBarSamples;

    // Round the precise values to the nearest sample.These values will be return for beat syncing.
    m_beatSamples.m_onetwentyeigthSamples = static_cast<int>(round(m_beatPrecise.m_onetwentyeigthSamplesPrecise));
    m_beatSamples.m_sixtyfourthSamples = static_cast<int>(round(m_beatPrecise.m_sixtyfourthSamplesPrecise));
    m_beatSamples.m_thirtysecondSamples = static_cast<int>(round(m_beatPrecise.m_thirtysecondSamplesPrecise));
    m_beatSamples.m_sixteenthSamples = static_cast<int>(round(m_beatPrecise.m_sixteenthSamplesPrecise));
    m_beatSamples.m_eigthSamples = static_cast<int>(round(m_beatPrecise.m_eigthSamplesPrecise));
    m_beatSamples.m_quarterSamples = static_cast<int>(round(m_beatPrecise.m_quarterSamplesPrecise));
    m_beatSamples.m_halfSamples = static_cast<int>(round(m_beatPrecise.m_halfSamplesPrecise));
    m_beatSamples.m_wholeSamples = static_cast<int>(round(m_beatPrecise.m_wholeSamplesPrecise));
    m_beatSamples.m_barSamples = static_cast<int>(round(m_beatPrecise.m_barSamplesPrecise));

    // Use the rounded sample values to calculate the dotted times. Note that dotted time means the
    // current notes value to the value of the previous one.
    m_beatSamples.m_sixtyfourthDottedSamples = m_beatSamples.m_sixtyfourthSamples + m_beatSamples.m_onetwentyeigthSamples;
    m_beatSamples.m_thirtysecondDottedSamples = m_beatSamples.m_thirtysecondSamples + m_beatSamples.m_sixtyfourthSamples;
    m_beatSamples.m_sixteenthDottedSamples = m_beatSamples.m_sixteenthSamples + m_beatSamples.m_thirtysecondSamples;
    m_beatSamples.m_eigthDottedSamples = m_beatSamples.m_eigthSamples + m_beatSamples.m_sixteenthSamples;
    m_beatSamples.m_quarterDottedSamples = m_beatSamples.m_quarterSamples + m_beatSamples.m_eigthSamples;
    m_beatSamples.m_halfDottedSamples = m_beatSamples.m_halfSamples + m_beatSamples.m_quarterSamples;

    // Use the precise sample values to calculate triplet values. Note a triplet takes two notes and
    // divides it into three.
    m_beatSamples.m_sixtyfourthTripletSamples = static_cast<int>(round((m_beatPrecise.m_sixtyfourthSamplesPrecise * 2.0) / 3.0));
    m_beatSamples.m_thirtysecondTripletSamples = static_cast<int>(round((m_beatPrecise.m_thirtysecondSamplesPrecise * 2.0) / 3.0));
    m_beatSamples.m_sixteenthTripletSamples = static_cast<int>(round((m_beatPrecise.m_sixteenthSamplesPrecise * 2.0) / 3.0));
    m_beatSamples.m_eigthTripletSamples = static_cast<int>(round((m_beatPrecise.m_eigthSamplesPrecise * 2.0) / 3.0));
    m_beatSamples.m_quarterTripletSamples = static_cast<int>(round((m_beatPrecise.m_quarterSamplesPrecise * 2.0) / 3.0));
    m_beatSamples.m_halfTripletSamples = static_cast<int>(round((m_beatPrecise.m_halfSamplesPrecise * 2.0) / 3.0));

    // Used for updating the beat counter. Provides the number of sample of a beat.
    // Note that the number of samples per beat depends on meter (bottom). Hence, this
    // switch.
    double samplesPerBeat = 0;
    switch (meter.m_bottom)
    {
        case 1: samplesPerBeat = m_beatPrecise.m_wholeSamplesPrecise; break;
        case 2: samplesPerBeat = m_beatPrecise.m_halfSamplesPrecise; break;
        case 4: samplesPerBeat = m_beatPrecise.m_quarterSamplesPrecise; break;
        case 8: samplesPerBeat = m_beatPrecise.m_eigthSamplesPrecise; break;
        case 16: samplesPerBeat = m_beatPrecise.m_sixteenthSamplesPrecise; break;
        case 32: samplesPerBeat = m_beatPrecise.m_thirtysecondSamplesPrecise; break;
        case 64: samplesPerBeat = m_beatPrecise.m_sixtyfourthSamplesPrecise; break;
        default: RF_FAIL("Beat division not supported."); break;
    }

    return samplesPerBeat;
}

int rf::BeatCalculator::BeatSwitch(const Sync& sync) const
{
    switch (sync.m_sync)
    {
        case Sync::Value::OneTwentyEigth: return static_cast<int>(round(m_beatPrecise.m_onetwentyeigthSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::SixtyFourth: return static_cast<int>(round(m_beatPrecise.m_sixtyfourthSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::ThirtySecond: return static_cast<int>(round(m_beatPrecise.m_thirtysecondSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::Sixteenth: return static_cast<int>(round(m_beatPrecise.m_sixteenthSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::Eigth: return static_cast<int>(round(m_beatPrecise.m_eigthSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::Quarter: return static_cast<int>(round(m_beatPrecise.m_quarterSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::Half: return static_cast<int>(round(m_beatPrecise.m_halfSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::Whole: return static_cast<int>(round(m_beatPrecise.m_wholeSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::Bar: return static_cast<int>(round(m_beatPrecise.m_barSamplesPrecise * sync.m_factor)); break;
        case Sync::Value::SixtyFourthDotted: return m_beatSamples.m_sixtyfourthDottedSamples * sync.m_factor; break;
        case Sync::Value::ThirtySecondDotted: return m_beatSamples.m_thirtysecondDottedSamples * sync.m_factor; break;
        case Sync::Value::SixteenthDotted: return m_beatSamples.m_sixteenthDottedSamples * sync.m_factor; break;
        case Sync::Value::EigthDotted: return m_beatSamples.m_eigthDottedSamples * sync.m_factor; break;
        case Sync::Value::QuarterDotted: return m_beatSamples.m_quarterDottedSamples * sync.m_factor; break;
        case Sync::Value::HalfDotted: return m_beatSamples.m_halfDottedSamples * sync.m_factor; break;
        case Sync::Value::SixtyFourthTriplet: return m_beatSamples.m_sixtyfourthTripletSamples * sync.m_factor; break;
        case Sync::Value::ThirtySecondTriplet: return m_beatSamples.m_thirtysecondTripletSamples * sync.m_factor; break;
        case Sync::Value::SixteenthTriplet: return m_beatSamples.m_sixteenthTripletSamples * sync.m_factor; break;
        case Sync::Value::EigthTriplet: return m_beatSamples.m_eigthTripletSamples * sync.m_factor; break;
        case Sync::Value::QuarterTriplet: return m_beatSamples.m_quarterTripletSamples * sync.m_factor; break;
        case Sync::Value::HalfTriplet: return m_beatSamples.m_halfTripletSamples * sync.m_factor; break;
        case Sync::Value::Cut: return 1; break;
        default:
            RF_FAIL("Could not find beat type in CalculateStartTime().");
            return 1;
            break;
    }
}

double rf::BeatCalculator::PreciseBeatSwitch(const Sync& sync) const
{
    switch (sync.m_sync)
    {
        case Sync::Value::OneTwentyEigth: return m_beatPrecise.m_onetwentyeigthSamplesPrecise * sync.m_factor; break;
        case Sync::Value::SixtyFourth: return m_beatPrecise.m_sixtyfourthSamplesPrecise * sync.m_factor; break;
        case Sync::Value::ThirtySecond: return m_beatPrecise.m_thirtysecondSamplesPrecise * sync.m_factor; break;
        case Sync::Value::Sixteenth: return m_beatPrecise.m_sixteenthSamplesPrecise * sync.m_factor; break;
        case Sync::Value::Eigth: return m_beatPrecise.m_eigthSamplesPrecise * sync.m_factor; break;
        case Sync::Value::Quarter: return m_beatPrecise.m_quarterSamplesPrecise * sync.m_factor; break;
        case Sync::Value::Half: return m_beatPrecise.m_halfSamplesPrecise * sync.m_factor; break;
        case Sync::Value::Whole: return m_beatPrecise.m_wholeSamplesPrecise * sync.m_factor; break;
        case Sync::Value::Bar: return m_beatPrecise.m_barSamplesPrecise * sync.m_factor; break;
        case Sync::Value::SixtyFourthDotted: return m_beatSamples.m_sixtyfourthDottedSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::ThirtySecondDotted: return m_beatSamples.m_thirtysecondDottedSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::SixteenthDotted: return m_beatSamples.m_sixteenthDottedSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::EigthDotted: return m_beatSamples.m_eigthDottedSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::QuarterDotted: return m_beatSamples.m_quarterDottedSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::HalfDotted: return m_beatSamples.m_halfDottedSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::SixtyFourthTriplet: return m_beatSamples.m_sixtyfourthTripletSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::ThirtySecondTriplet: return m_beatSamples.m_thirtysecondTripletSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::SixteenthTriplet: return m_beatSamples.m_sixteenthTripletSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::EigthTriplet: return m_beatSamples.m_eigthTripletSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::QuarterTriplet: return m_beatSamples.m_quarterTripletSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::HalfTriplet: return m_beatSamples.m_halfTripletSamples * sync.m_factor * 1.0f; break;
        case Sync::Value::Cut: return 1.0f; break;
        default:
            RF_FAIL("Could not find beat type in CalculateStartTime().");
            return 1.0f;
            break;
    }
}