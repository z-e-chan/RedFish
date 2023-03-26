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

#include "editorconvolverplugin.h"

#include <imgui.h>
#include <redfish/convolverplugin.h>
#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>
#include <redfish/pluginutils.h>

#include "actionhandler.h"
#include "pluginactions.h"

rf::EditorConvolverPlugin::EditorConvolverPlugin(EditorPlugin::Type type)
    : EditorPlugin(type)
{
}

void rf::EditorConvolverPlugin::Edit(bool* open, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, ActionHandler* actions)
{
    RF_EDITOR_PLUGIN_GUI_BEGIN(ConvolverPlugin);

    ImGui::Text("Coming Soon!");

    RF_EDITOR_PLUGIN_GUI_END;
}

void rf::EditorConvolverPlugin::InitializeState(void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    // TODO
    buffer;
    mixerSystem;
    mixGroupHandle;
    slot;
}

void rf::EditorConvolverPlugin::SetFromState(const void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot)
{
    // TODO
    buffer;
    mixerSystem;
    mixGroupHandle;
    slot;
}
