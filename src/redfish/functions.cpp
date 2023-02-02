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

#include "functions.h"

#include <random>

#include "basevoice.h"
#include "messenger.h"

bool rf::Functions::FloatEquality(float x, float y)
{
    const float epsilon = 1.192092896e-07F;
    return (x >= y ? x - y : y - x) < epsilon;
}

int rf::Functions::Clamp(int x, int min, int max)
{
    return std::max(min, std::min(x, max));
}

float rf::Functions::Clamp(float x, float min, float max)
{
    return std::max(min, std::min(x, max));
}

float rf::Functions::Lerp(float percent, float start, float end)
{
    percent = Clamp(percent, 0.0f, 1.0f);
    return (percent * end) + ((1.0f - percent) * start);
}

float rf::Functions::Map(float x, float inMin, float inMax, float outMin, float outMax)
{
    const float percent = Clamp((x - inMin) / (inMax - inMin), 0.0f, 1.0f);
    return Lerp(percent, outMin, outMax);
}

int rf::Functions::RandomInt(int min, int max)
{
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(min, max);
    return distr(gen);
}

float rf::Functions::RandomFloat(float min, float max)
{
    const float random = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min));
    return Clamp(random, min, max);
}

float rf::Functions::DecibelToAmplitude(float decibel)
{
    // Special cases to avoid rounding errors.
    if (Functions::FloatEquality(decibel, 0.0f))
    {
        return 1.0f;
    }
    if (decibel <= RF_MIN_DECIBELS)
    {
        return 0.0f;
    }

    return powf(10.0f, 0.05f * decibel);
}

float rf::Functions::AmplitudeToDecibel(float amplitude)
{
    if (amplitude <= 0.0f)
    {
        return RF_MIN_DECIBELS;
    }

    return 20.0f * log10f(amplitude);
}

float rf::Functions::ComputePeakDecibel(float* sampleBuffer, int bufferSize)
{
    float maxValue = 0.0f;

    for (int i = 0; i < bufferSize; ++i)
    {
        maxValue = std::max(maxValue, std::abs(sampleBuffer[i]));
    }

    return 20.0f * log10f(maxValue);
}

float rf::Functions::ComputePeakAmplitude(float* sampleBuffer, int bufferSize)
{
    float maxValue = 0.0f;

    for (int i = 0; i < bufferSize; ++i)
    {
        maxValue = std::max(maxValue, std::abs(sampleBuffer[i]));
    }

    return maxValue;
}

float rf::Functions::ComputeRMSDecibel(float* sampleBuffer, int bufferSize)
{
    float sum = 0.0f;

    for (int i = 0; i < bufferSize; ++i)
    {
        sum += sampleBuffer[i] * sampleBuffer[i];
    }

    sum = sqrtf((1.0f / bufferSize) * sum);

    return 20.0f * log10f(sum);
}

float rf::Functions::ComputeRMSAmplitude(float* sampleBuffer, int bufferSize)
{
    float sum = 0.0f;

    for (int i = 0; i < bufferSize; ++i)
    {
        sum += sampleBuffer[i] * sampleBuffer[i];
    }

    return sqrtf((1.0f / bufferSize) * sum);
}

float rf::Functions::Int16ToFloat32(int16_t sample)
{
    constexpr float positiveScale = 1.0f / 32767.0f;
    constexpr float negativeScale = 1.0f / 32768.0f;
    return sample >= 0 ? sample * positiveScale : sample * negativeScale;
}

int rf::Functions::MsToSamples(float ms, int sampleRate)
{
    return static_cast<int>(sampleRate * (ms * 0.001f));
}

bool rf::Functions::InFirstWindow(long long playhead, long long startTime, int bufferSize)
{
    return (startTime >= playhead) && (startTime < (playhead + bufferSize));
}

void rf::Functions::SendVoiceStartMessage(const BaseVoice& voice, Messenger* messanger)
{
    Message msg;
    msg.m_type = MessageType::ContextVoiceStart;
    Message::ContextVoiceStartData* data = msg.GetContextVoiceStartData();
    data->m_audioHandle = voice.GetAudioHandle();
    messanger->AddMessage(msg);
}

void rf::Functions::SendVoiceStopMessage(const BaseVoice& voice, Messenger* messanger)
{
    Message msg;
    msg.m_type = MessageType::ContextVoiceStop;
    Message::ContextVoiceStopData* data = msg.GetContextVoiceStopData();
    data->m_audioHandle = voice.GetAudioHandle();
    messanger->AddMessage(msg);
}

size_t rf::Functions::HashString(const char* str)
{
    size_t h = 5381;
    int c = 0;
    while ((c = *str++))
    {
        h = ((h << 5) + h) + c;
    }
    return h;
}