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

#include "positioningplugin.h"

#include "commandprocessor.h"
#include "functions.h"
#include "plugincommands.h"
#include "pluginutils.h"

rf::PositioningPlugin::PositioningPlugin(Context* context,
                                         CommandProcessor* commands,
                                         MixGroupHandle mixGroupHandle,
                                         int mixGroupSlot,
                                         int pluginIndex)
    : PluginBase(context, commands, mixGroupHandle, mixGroupSlot, pluginIndex, PluginBase::Type::Positioning)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreatePositioningDSPCommand);
}

rf::PositioningPlugin::~PositioningPlugin()
{
    RF_SEND_PLUGIN_DESTROY_COMMAND(DestroyPositioningDSPCommand);
}

void rf::PositioningPlugin::SetPositioningParameters(const PositioningParameters& parameters)
{
    if (m_positioningParameters == parameters)
    {
        return;
    }

    m_positioningParameters = parameters;
    m_positioningParameters.m_panAngle = Functions::Clamp(m_positioningParameters.m_panAngle, -1.0f, 1.0f);
    m_positioningParameters.m_maxHpfCutoff =
        Functions::Clamp(m_positioningParameters.m_maxHpfCutoff, PluginUtils::k_minFilterCutoff, PluginUtils::k_maxFilterCutoff);
    m_positioningParameters.m_maxLpfCutoff =
        Functions::Clamp(m_positioningParameters.m_maxLpfCutoff, PluginUtils::k_minFilterCutoff, PluginUtils::k_maxFilterCutoff);

    AudioCommand cmd;
    SetPositioningDSPPositioningParametersCommand& data = EncodeAudioCommand<SetPositioningDSPPositioningParametersCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_parameters = m_positioningParameters;
    m_commands->Add(cmd);
}

const rf::PositioningParameters& rf::PositioningPlugin::GetPositioningParameters() const
{
    return m_positioningParameters;
}
void rf::PositioningPlugin::ToJson(nlohmann::ordered_json& json) const
{
    json["distanceCurveType"] = m_positioningParameters.m_distanceCurveType;
    json["panAngle"] = m_positioningParameters.m_panAngle;
    json["currentDistance"] = m_positioningParameters.m_currentDistance;
    json["minDistance"] = m_positioningParameters.m_minDistance;
    json["maxDistance"] = m_positioningParameters.m_maxDistance;
    json["maxAttenuationDb"] = m_positioningParameters.m_maxAttenuationDb;
    json["maxHpfCutoff"] = m_positioningParameters.m_maxHpfCutoff;
    json["maxLpfCutoff"] = m_positioningParameters.m_maxLpfCutoff;
    json["enable"] = m_positioningParameters.m_enable;
}

void rf::PositioningPlugin::FromJson(const nlohmann::ordered_json& json)
{
    m_positioningParameters.m_distanceCurveType = json.value("distanceCurveType", PositioningParameters::DistanceCurve::Linear);
    m_positioningParameters.m_panAngle = json.value("panAngle", 0.0f);
    m_positioningParameters.m_currentDistance = json.value("currentDistance", 0.0f);
    m_positioningParameters.m_minDistance = json.value("minDistance", 0.0f);
    m_positioningParameters.m_maxDistance = json.value("maxDistance", 0.0f);
    m_positioningParameters.m_maxAttenuationDb = json.value("maxAttenuationDb", 0.0f);
    m_positioningParameters.m_maxHpfCutoff = json.value("maxHpfCutoff", 20.0f);
    m_positioningParameters.m_maxLpfCutoff = json.value("maxLpfCutoff", 20000.0f);
    m_positioningParameters.m_enable = json.value("enable", false);
}