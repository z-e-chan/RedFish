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
class BaseVoice;
class Messenger;
}  // namespace rf

namespace rf::Functions
{
bool FloatEquality(float x, float y);
int Clamp(int x, int min, int max);
float Clamp(float x, float min, float max);
float Lerp(float percent, float start, float end);
float Map(float x, float inMin, float inMax, float outMin, float outMax);
int RandomInt(int min, int max);
float RandomFloat(float min, float max);
float DecibelToAmplitude(float decibel);
float AmplitudeToDecibel(float amplitude);
float ComputePeakDecibel(float* sampleBuffer, int bufferSize);
float ComputePeakAmplitude(float* sampleBuffer, int bufferSize);
float ComputeRMSDecibel(float* sampleBuffer, int bufferSize);
float ComputeRMSAmplitude(float* sampleBuffer, int bufferSize);
float Int16ToFloat32(int16_t sample);
int MsToSamples(float ms, int sampleRate);
bool InFirstWindow(long long playhead, long long startTime, int bufferSize);
void SendVoiceStartMessage(const BaseVoice& voice, Messenger* messanger);
void SendVoiceStopMessage(const BaseVoice& voice, Messenger* messanger);
size_t HashString(const char* str);
}  // namespace rf::Functions