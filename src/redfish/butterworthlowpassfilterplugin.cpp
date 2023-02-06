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

#include "butterworthlowpassfilterplugin.h"

#include "commandprocessor.h"
#include "functions.h"
#include "plugincommands.h"

rf::ButterworthLowpassFilterPlugin::ButterworthLowpassFilterPlugin(Context* context,
                                                                   CommandProcessor* commands,
                                                                   MixGroupHandle mixGroupHandle,
                                                                   int mixGroupSlot,
                                                                   int pluginIndex)
    : PluginBase(context, commands, mixGroupHandle, mixGroupSlot, pluginIndex, PluginBase::Type::ButterworthLowpassFilter)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreateButterworthLowpassFilterDSPCommand);
}

rf::ButterworthLowpassFilterPlugin::~ButterworthLowpassFilterPlugin()
{
    RF_SEND_PLUGIN_DESTROY_COMMAND(DestroyButterworthLowpassFilterDSPCommand);
}

void rf::ButterworthLowpassFilterPlugin::SetOrder(int order)
{
    if (m_order == order)
    {
        return;
    }

    m_order = Functions::Clamp(order, 1, 2);

    AudioCommand cmd;
    SetButterworthLowpassFilterDSPOrderCommand& data = EncodeAudioCommand<SetButterworthLowpassFilterDSPOrderCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_order = m_order;
    m_commands->Add(cmd);
}

int rf::ButterworthLowpassFilterPlugin::GetOrder() const
{
    return m_order;
}

void rf::ButterworthLowpassFilterPlugin::SetCutoff(float cutoff)
{
    if (Functions::FloatEquality(m_cutoff, cutoff))
    {
        return;
    }

    m_cutoff = Functions::Clamp(cutoff, PluginUtils::k_minFilterCutoff, PluginUtils::k_maxFilterCutoff);

    AudioCommand cmd;
    SetButterworthLowpassFilterDSPCutoffCommand& data = EncodeAudioCommand<SetButterworthLowpassFilterDSPCutoffCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_cutoff = m_cutoff;
    m_commands->Add(cmd);
}

float rf::ButterworthLowpassFilterPlugin::GetCutoff() const
{
    return m_cutoff;
}

void rf::ButterworthLowpassFilterPlugin::ToJson(nlohmann::ordered_json& json) const
{
    json["order"] = GetOrder();
    json["cutoff"] = GetCutoff();
}

void rf::ButterworthLowpassFilterPlugin::FromJson(const nlohmann::ordered_json& json)
{
    SetOrder(json.value("order", GetOrder()));
    SetCutoff(json.value("cutoff", GetCutoff()));
}