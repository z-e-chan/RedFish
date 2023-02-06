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

#include "iir2highpassfilterplugin.h"

#include "commandprocessor.h"
#include "functions.h"
#include "plugincommands.h"

rf::IIR2HighpassFilterPlugin::IIR2HighpassFilterPlugin(Context* context,
                                                       CommandProcessor* commands,
                                                       MixGroupHandle mixGroupHandle,
                                                       int mixGroupSlot,
                                                       int pluginIndex)
    : PluginBase(context, commands, mixGroupHandle, mixGroupSlot, pluginIndex, PluginBase::Type::IIR2HighpassFilter)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreateIIR2HighpassFilterDSPCommand);
}

rf::IIR2HighpassFilterPlugin::~IIR2HighpassFilterPlugin()
{
    RF_SEND_PLUGIN_DESTROY_COMMAND(DestroyIIR2HighpassFilterDSPCommand);
}

void rf::IIR2HighpassFilterPlugin::SetQ(float q)
{
    if (Functions::FloatEquality(m_q, q))
    {
        return;
    }

    m_q = Functions::Clamp(q, PluginUtils::k_minFilterQ, PluginUtils::k_maxFilterQ);

    AudioCommand cmd;
    SetIIR2HighpassFilterDSPQCommand& data = EncodeAudioCommand<SetIIR2HighpassFilterDSPQCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_q = m_q;
    m_commands->Add(cmd);
}

float rf::IIR2HighpassFilterPlugin::GetQ() const
{
    return m_q;
}

void rf::IIR2HighpassFilterPlugin::SetCutoff(float cutoff)
{
    if (Functions::FloatEquality(m_cutoff, cutoff))
    {
        return;
    }

    m_cutoff = Functions::Clamp(cutoff, PluginUtils::k_minFilterCutoff, PluginUtils::k_maxFilterCutoff);

    AudioCommand cmd;
    SetIIR2HighpassFilterDSPCutoffCommand& data = EncodeAudioCommand<SetIIR2HighpassFilterDSPCutoffCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_cutoff = m_cutoff;
    m_commands->Add(cmd);
}

float rf::IIR2HighpassFilterPlugin::GetCutoff() const
{
    return m_cutoff;
}

void rf::IIR2HighpassFilterPlugin::ToJson(nlohmann::ordered_json& json) const
{
    json["q"] = GetQ();
    json["cutoff"] = GetCutoff();
}

void rf::IIR2HighpassFilterPlugin::FromJson(const nlohmann::ordered_json& json)
{
    SetQ(json.value("q", GetQ()));
    SetCutoff(json.value("cutoff", GetCutoff()));
}