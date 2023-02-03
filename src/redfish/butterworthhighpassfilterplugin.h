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
#include "pluginutils.h"

namespace rf
{
class ButterworthHighpassFilterPlugin : public PluginBase
{
public:
    ButterworthHighpassFilterPlugin(Context* context, CommandProcessor* commands, MixGroupHandle mixGroupHandle, int mixGroupSlot, int pluginIndex);
    ButterworthHighpassFilterPlugin(const ButterworthHighpassFilterPlugin&) = delete;
    ButterworthHighpassFilterPlugin(ButterworthHighpassFilterPlugin&&) = delete;
    ButterworthHighpassFilterPlugin& operator=(const ButterworthHighpassFilterPlugin&) = delete;
    ButterworthHighpassFilterPlugin& operator=(ButterworthHighpassFilterPlugin&&) = delete;
    ~ButterworthHighpassFilterPlugin();

    void SetOrder(int order);
    int GetOrder() const;
    void SetCutoff(float cutoff);
    float GetCutoff() const;

private:
    int m_order = 1;
    float m_cutoff = PluginUtils::k_minFilterCutoff;
};
}  // namespace rf