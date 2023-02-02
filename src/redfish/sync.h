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

namespace rf
{
struct Sync
{
    enum class Mode : short
    {
        Time,
        Musical
    };

    enum class ReferencePoint : short
    {
        Bar,
        CueStart
    };

    enum class Value : short
    {
        Cut,
        Queue,
        Bar,
        Whole,
        Half,
        Quarter,
        Eigth,
        Sixteenth,
        ThirtySecond,
        SixtyFourth,
        HalfDotted,
        QuarterDotted,
        EigthDotted,
        SixteenthDotted,
        ThirtySecondDotted,
        SixtyFourthDotted,
        HalfTriplet,
        QuarterTriplet,
        EigthTriplet,
        SixteenthTriplet,
        ThirtySecondTriplet,
        SixtyFourthTriplet,
        OneTwentyEigth
    };

    // The mode of the sync, either Time or Musical.
    Mode m_mode = Mode::Time;
    // The reference from which we calculate the music transition.
    // Bar: we calculate the transition from the current bar
    // CueStart: we calculate the transition from the start of the current music cue
    ReferencePoint m_referencePoint = ReferencePoint::Bar;
    // The musical value of the sync.
    Value m_sync = Value::Cut;

    // The factor of the musical value for the sync. For instance, 3 quarter notes.
    int m_factor = 1;
    // The amount of time in the case of Time mode syncs.
    float m_time = 0.0f;

    Sync() = default;
    Sync(Value syncValue);
    Sync(Value syncValue, int factor);
    Sync(float time);
};
}  // namespace rf