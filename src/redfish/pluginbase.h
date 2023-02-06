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
#include <external/nlohmann/json.hpp>

#include "identifiers.h"

namespace rf
{
class CommandProcessor;
class Context;

class PluginBase
{
public:
    enum class Type
    {
        Invalid,
        ButterworthHighpassFilter,
        ButterworthLowpassFilter,
        Compressor,
        Convolver,
        Delay,
        Gain,
        IIR2HighpassFilter,
        IIR2LowpassFilter,
        Limiter,
        Pan,
        Positioning,

        Version = 1,
    };

    PluginBase(Context* context, CommandProcessor* commands, MixGroupHandle mixGroupHandle, int mixGroupSlot, int pluginIndex, Type type);
    PluginBase(const PluginBase&) = delete;
    PluginBase(PluginBase&&) = delete;
    PluginBase& operator=(const PluginBase&) = delete;
    PluginBase& operator=(PluginBase&&) = delete;
    virtual ~PluginBase() = default;

    PluginHandle GetPluginHandle() const;
    Type GetType() const;
    void SetBypass(bool bypass);
    bool GetBypass() const;

    virtual void ToJson(nlohmann::ordered_json& json) const = 0;
    virtual void FromJson(const nlohmann::ordered_json& json) = 0;

protected:
    Context* m_context = nullptr;
    CommandProcessor* m_commands = nullptr;
    PluginHandle m_pluginHandle;
    MixGroupHandle m_mixGroupHandle;
    Type m_type = Type::Invalid;
    int m_mixGroupSlot = -1;
    int m_pluginIndex = -1;
    bool m_bypass = false;
};
}  // namespace rf