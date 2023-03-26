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

#include "editorgainplugin.h"

#include <imgui.h>
#include <redfish/gainplugin.h>
#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>

#include "actionhandler.h"
#include "pluginactions.h"

rf::EditorGainPlugin::EditorGainPlugin(EditorPlugin::Type type)
    : EditorPlugin(type)
{
}

void rf::EditorGainPlugin::Edit(bool* open, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, ActionHandler* actions)
{
    RF_EDITOR_PLUGIN_GUI_BEGIN(GainPlugin);

    RF_PLUGIN_GUI_FLOAT_SLIDER("Gain dB", 0.01f, RF_MIN_DECIBELS, RF_MAX_DECIBELS, plugin->GetGainDb, plugin->SetGainDb, Gain_SetGain);

    RF_EDITOR_PLUGIN_GUI_END;
}

void rf::EditorGainPlugin::InitializeState(void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_INIT_STATE(GainPlugin);
    state->m_gainDb = plugin->GetGainDb();
}

void rf::EditorGainPlugin::SetFromState(const void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_SET_FROM_STATE(GainPlugin);
    plugin->SetGainDb(state->m_gainDb);
}