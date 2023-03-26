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

#include "iir2lowpassfilterplugin.h"

#include "commandprocessor.h"
#include "functions.h"
#include "plugincommands.h"

rf::IIR2LowpassFilterPlugin::IIR2LowpassFilterPlugin(Context* context,
                                                     CommandProcessor* commands,
                                                     MixGroupHandle mixGroupHandle,
                                                     int mixGroupSlot,
                                                     int pluginIndex)
    : PluginBase(context, commands, mixGroupHandle, mixGroupSlot, pluginIndex, PluginBase::Type::IIR2LowpassFilter)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreateIIR2LowpassFilterDSPCommand);
}

rf::IIR2LowpassFilterPlugin::~IIR2LowpassFilterPlugin()
{
    RF_SEND_PLUGIN_DESTROY_COMMAND(DestroyIIR2LowpassFilterDSPCommand);
}

void rf::IIR2LowpassFilterPlugin::SetQ(float q)
{
    if (Functions::FloatEquality(m_q, q))
    {
        return;
    }

    m_q = Functions::Clamp(q, PluginUtils::k_minFilterQ, PluginUtils::k_maxFilterQ);

    AudioCommand cmd;
    SetIIR2LowpassFilterDSPQCommand& data = EncodeAudioCommand<SetIIR2LowpassFilterDSPQCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_q = m_q;
    m_commands->Add(cmd);
}

float rf::IIR2LowpassFilterPlugin::GetQ() const
{
    return m_q;
}

void rf::IIR2LowpassFilterPlugin::SetCutoff(float cutoff)
{
    if (Functions::FloatEquality(m_cutoff, cutoff))
    {
        return;
    }

    m_cutoff = Functions::Clamp(cutoff, PluginUtils::k_minFilterCutoff, PluginUtils::k_maxFilterCutoff);

    AudioCommand cmd;
    SetIIR2LowpassFilterDSPCutoffCommand& data = EncodeAudioCommand<SetIIR2LowpassFilterDSPCutoffCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_cutoff = m_cutoff;
    m_commands->Add(cmd);
}

float rf::IIR2LowpassFilterPlugin::GetCutoff() const
{
    return m_cutoff;
}

void rf::IIR2LowpassFilterPlugin::ToJson(nlohmann::ordered_json& json) const
{
    json["q"] = GetQ();
    json["cutoff"] = GetCutoff();
}

void rf::IIR2LowpassFilterPlugin::FromJson(const nlohmann::ordered_json& json)
{
    SetQ(json.value("q", GetQ()));
    SetCutoff(json.value("cutoff", GetCutoff()));
}