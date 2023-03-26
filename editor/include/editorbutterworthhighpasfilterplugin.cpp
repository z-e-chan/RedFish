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

#include "editorbutterworthhighpasfilterplugin.h"

#include <imgui.h>
#include <redfish/butterworthhighpassfilterplugin.h>
#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>

#include "actionhandler.h"
#include "pluginactions.h"

rf::EditorButterworthHighpassFilterPlugin::EditorButterworthHighpassFilterPlugin(EditorPlugin::Type type)
    : EditorPlugin(type)
{
}

void rf::EditorButterworthHighpassFilterPlugin::Edit(bool* open,
                                                     MixerSystem* mixerSystem,
                                                     MixGroupHandle mixGroupHandle,
                                                     int slot,
                                                     ActionHandler* actions)
{
    RF_EDITOR_PLUGIN_GUI_BEGIN(ButterworthHighpassFilterPlugin);

    RF_PLUGIN_GUI_INT_SLIDER("Order", 1, 1, 2, plugin->GetOrder, plugin->SetOrder, BWHP_SetOrder);
    RF_PLUGIN_GUI_FLOAT_SLIDER(
        "Cutoff", 10.0f, PluginUtils::k_minFilterCutoff, PluginUtils::k_maxFilterCutoff, plugin->GetCutoff, plugin->SetCutoff, BWHP_SetCutoff);

    RF_EDITOR_PLUGIN_GUI_END;
}

void rf::EditorButterworthHighpassFilterPlugin::InitializeState(void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_INIT_STATE(ButterworthHighpassFilterPlugin);
    state->m_order = plugin->GetOrder();
    state->m_cutoff = plugin->GetCutoff();
}

void rf::EditorButterworthHighpassFilterPlugin::SetFromState(const void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_SET_FROM_STATE(ButterworthHighpassFilterPlugin);
    plugin->SetOrder(state->m_order);
    plugin->SetCutoff(state->m_cutoff);
}