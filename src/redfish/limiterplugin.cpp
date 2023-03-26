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

#include "limiterplugin.h"

#include "commandprocessor.h"
#include "functions.h"
#include "plugincommands.h"
#include "pluginutils.h"

rf::LimiterPlugin::LimiterPlugin(Context* context, CommandProcessor* commands, MixGroupHandle mixGroupHandle, int mixGroupSlot, int pluginIndex)
    : PluginBase(context, commands, mixGroupHandle, mixGroupSlot, pluginIndex, PluginBase::Type::Limiter)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreateLimiterDSPCommand);
}

rf::LimiterPlugin::~LimiterPlugin()
{
    RF_SEND_PLUGIN_DESTROY_COMMAND(DestroyLimiterDSPCommand);
}

void rf::LimiterPlugin::SetThreshold(float threshold)
{
    if (Functions::FloatEquality(m_threshold, threshold))
    {
        return;
    }

    m_threshold = threshold;

    AudioCommand cmd;
    SetLimiterDSPThresholdCommand& data = EncodeAudioCommand<SetLimiterDSPThresholdCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_threshold = m_threshold;
    m_commands->Add(cmd);
}

float rf::LimiterPlugin::GetThreshold() const
{
    return m_threshold;
}

void rf::LimiterPlugin::ToJson(nlohmann::ordered_json& json) const
{
    json["threshold"] = GetThreshold();
}

void rf::LimiterPlugin::FromJson(const nlohmann::ordered_json& json)
{
    SetThreshold(json.value("threshold", GetThreshold()));
}