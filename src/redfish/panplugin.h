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
class PanPlugin : public PluginBase
{
public:
    PanPlugin(Context* context, CommandProcessor* commands, int pluginIndex, int mixGroupSlot, MixGroupHandle mixGroupHandle);
    PanPlugin(const PanPlugin&) = delete;
    PanPlugin(PanPlugin&&) = delete;
    PanPlugin& operator=(const PanPlugin&) = delete;
    PanPlugin& operator=(PanPlugin&&) = delete;
    ~PanPlugin() = default;

    void SetAngle(float angle);
    float GetAngle() const;

private:
    float m_angle = 0.0;
};
}  // namespace rf