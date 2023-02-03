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
#include "pluginbase.h"

namespace rf
{
class CompressorPlugin : public PluginBase
{
public:
    CompressorPlugin(Context* context, CommandProcessor* commands, MixGroupHandle mixGroupHandle, int mixGroupSlot, int pluginIndex);
    CompressorPlugin(const CompressorPlugin&) = delete;
    CompressorPlugin(CompressorPlugin&&) = delete;
    CompressorPlugin& operator=(const CompressorPlugin&) = delete;
    CompressorPlugin& operator=(CompressorPlugin&&) = delete;
    ~CompressorPlugin();

    void SetThreshold(float threshold);
    float GetThreshold() const;
    void SetRatio(float ratio);
    float GetRatio() const;
    void SetMakeUpGainDb(float makeUpGainDb);
    float GetMakeUpGainDb() const;
    void SetAttack(float attack);
    float GetAttack() const;
    void SetRelease(float release);
    float GetRelease() const;

private:
    float m_threshold = -24.0f;
    float m_ratio = 1.0f;
    float m_makeUpGainDb = 0.0f;
    float m_attack = 10.0f;
    float m_release = 300.0f;
};
}  // namespace rf