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

#include "editorcompressorplugin.h"

#include <imgui.h>
#include <redfish/compressorplugin.h>
#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>

#include "actionhandler.h"
#include "pluginactions.h"

rf::EditorCompressorPlugin::EditorCompressorPlugin(EditorPlugin::Type type)
    : EditorPlugin(type)
{
}

void rf::EditorCompressorPlugin::Edit(bool* open, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, ActionHandler* actions)
{
    RF_EDITOR_PLUGIN_GUI_BEGIN(CompressorPlugin);

    RF_PLUGIN_GUI_FLOAT_SLIDER(
        "Threshold", 0.01f, RF_MIN_DECIBELS, RF_MAX_DECIBELS, plugin->GetThreshold, plugin->SetThreshold, Compressor_SetThreshold);
    RF_PLUGIN_GUI_FLOAT_SLIDER("Ratio", 0.01f, 1.0f, 100.0f, plugin->GetRatio, plugin->SetRatio, Compressor_SetRatio);
    RF_PLUGIN_GUI_FLOAT_SLIDER(
        "Make Up Gain db", 0.01f, RF_MIN_DECIBELS, RF_MAX_DECIBELS, plugin->GetMakeUpGainDb, plugin->SetMakeUpGainDb, Compressor_SetMakeUpGain);
    RF_PLUGIN_GUI_FLOAT_SLIDER("Attack ms", 0.01f, 0.0f, 500.0f, plugin->GetAttack, plugin->SetAttack, Compressor_SetAttack);
    RF_PLUGIN_GUI_FLOAT_SLIDER("Release ms", 0.01f, 0.0f, 5000.0f, plugin->GetRelease, plugin->SetRelease, Compressor_SetRelease);

    RF_EDITOR_PLUGIN_GUI_END;
}

void rf::EditorCompressorPlugin::InitializeState(void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_INIT_STATE(CompressorPlugin);
    state->m_threshold = plugin->GetThreshold();
    state->m_ratio = plugin->GetRatio();
    state->m_makeUpGainDb = plugin->GetMakeUpGainDb();
    state->m_attackMs = plugin->GetAttack();
    state->m_releaseMs = plugin->GetRelease();
}

void rf::EditorCompressorPlugin::SetFromState(const void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    RF_SET_FROM_STATE(CompressorPlugin);
    plugin->SetThreshold(state->m_threshold);
    plugin->SetRatio(state->m_ratio);
    plugin->SetMakeUpGainDb(state->m_makeUpGainDb);
    plugin->SetAttack(state->m_attackMs);
    plugin->SetRelease(state->m_releaseMs);
}
