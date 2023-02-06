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

#pragma once
#include <redfish/identifiers.h>

#define RF_EDITOR_PLUGIN_GUI_BEGIN(pluginType)                     \
    char name[128] = {};                                           \
    BuildName(name, mixerSystem, mixGroupHandle, slot);            \
    ImGui::Begin(name, open, ImGuiWindowFlags_AlwaysAutoResize);   \
    MixGroup* mixGroup = mixerSystem->GetMixGroup(mixGroupHandle); \
    pluginType* plugin = mixGroup->GetPlugin<pluginType>(slot);    \
    RF_PLUGIN_GUI_BOOL("Bypass", plugin->GetBypass, SetBypassAction);

#define RF_EDITOR_PLUGIN_GUI_END ImGui::End();

#define RF_INIT_STATE(pluginType)                                     \
    if (!buffer)                                                      \
    {                                                                 \
        return;                                                       \
    }                                                                 \
    MixGroup* mixGroup = mixerSystem->GetMixGroup(mixGroupHandle);    \
    const pluginType* plugin = mixGroup->GetPlugin<pluginType>(slot); \
    State* state = reinterpret_cast<State*>(buffer);                  \
    state->m_bypass = plugin->GetBypass();

#define RF_SET_FROM_STATE(pluginType)                              \
    if (!buffer)                                                   \
    {                                                              \
        return;                                                    \
    }                                                              \
    MixGroup* mixGroup = mixerSystem->GetMixGroup(mixGroupHandle); \
    pluginType* plugin = mixGroup->GetPlugin<pluginType>(slot);    \
    const State* state = reinterpret_cast<const State*>(buffer);   \
    plugin->SetBypass(state->m_bypass);

namespace rf
{
class ActionHandler;
class PluginBase;
class MixerSystem;

class EditorPlugin
{
public:
    enum class Type
    {
        Invalid,
        ButterworthHighpassFilter,
        ButterworthLowpassFilter,
        Compressor,
        Convolver,
        Delay,
        Gain,
        IIR2HighpassFilter,
        IIR2LowpassFilter,
        Limiter,
        Pan,
        Positioning,

        Version = 1,
    };

    EditorPlugin(Type type);
    virtual ~EditorPlugin() = default;

    virtual void Edit(bool* open, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, ActionHandler* actions) = 0;
    virtual void InitializeState(void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot) = 0;
    virtual void SetFromState(const void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot) = 0;

    static const char* GetPluginName(Type type);
    const char* GetName() const;
    void BuildName(char* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot) const;

    Type m_type = Type::Invalid;
    PluginBase* m_plugin = nullptr;

    struct StateBase
    {
        bool m_bypass = false;
    };
};
}  // namespace rf