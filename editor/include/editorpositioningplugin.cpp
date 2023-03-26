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

#include "editorpositioningplugin.h"

#include <imgui.h>
#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>
#include <redfish/positioningplugin.h>

#include "actionhandler.h"
#include "pluginactions.h"

rf::EditorPositioningPlugin::EditorPositioningPlugin(EditorPlugin::Type type)
    : EditorPlugin(type)
{
}

void rf::EditorPositioningPlugin::Edit(bool* open, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, ActionHandler* actions)
{
    RF_EDITOR_PLUGIN_GUI_BEGIN(PositioningPlugin);

    bool active = false;
    bool end = false;
    PositioningParameters currentValue = plugin->GetPositioningParameters();
    static PositioningParameters s_firstValue;
    static bool s_first = true;
    PositioningParameters value = currentValue;

    active = ImGui::DragFloat("Pan", &value.m_panAngle, 0.001f, -1.0f, 1.0f) || active;
    end = ImGui::IsItemDeactivatedAfterEdit() || end;
    active = ImGui::DragFloat("Current Distance", &value.m_currentDistance, 1.0f, 0.0f, 1000.0f) || active;
    end = ImGui::IsItemDeactivatedAfterEdit() || end;
    active = ImGui::DragFloat("Min Distance", &value.m_minDistance, 1.0f, 0.0f, 1000.0f) || active;
    end = ImGui::IsItemDeactivatedAfterEdit() || end;
    active = ImGui::DragFloat("Max Distance", &value.m_maxDistance, 1.0f, 0.0f, 1000.0f) || active;
    end = ImGui::IsItemDeactivatedAfterEdit() || end;
    active = ImGui::DragFloat("Max Attenuation dB", &value.m_maxAttenuationDb, 1.0f, -60.0f, 12.0f) || active;
    end = ImGui::IsItemDeactivatedAfterEdit() || end;
    active = ImGui::DragFloat("Max HPF Cutoff", &value.m_maxHpfCutoff, 1.0f, 20.0f, 20000.0f) || active;
    end = ImGui::IsItemDeactivatedAfterEdit() || end;
    active = ImGui::DragFloat("Max LPF Cutoff", &value.m_maxLpfCutoff, 1.0f, 20.0f, 20000.0f) || active;
    end = ImGui::IsItemDeactivatedAfterEdit() || end;

    value.m_enable = true;
    plugin->SetPositioningParameters(value);

    if (active && s_first)
    {
        s_firstValue = currentValue;
        s_first = false;
    }
    if (end)
    {
        actions->DoAction(new SetPositioningParamsAction(this, mixerSystem, mixGroup->GetMixGroupHandle(), slot, s_firstValue, value));
        s_first = true;
    }

    RF_EDITOR_PLUGIN_GUI_END;
}

void rf::EditorPositioningPlugin::InitializeState(void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_INIT_STATE(PositioningPlugin);
    state->m_params = plugin->GetPositioningParameters();
}

void rf::EditorPositioningPlugin::SetFromState(const void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_SET_FROM_STATE(PositioningPlugin);
    plugin->SetPositioningParameters(state->m_params);
}