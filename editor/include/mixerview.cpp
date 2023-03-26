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

#include "mixerview.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <redfish/context.h>
#include <redfish/functions.h>
#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>
#include <redfish/pluginbase.h>
#include <redfish/send.h>
#include <redfish/version.h>

#include <fstream>

#include "actionhandler.h"
#include "deserializeddata.h"
#include "editorbutterworthhighpasfilterplugin.h"
#include "editorbutterworthlowpassfilterplugin.h"
#include "editorcompressorplugin.h"
#include "editorconvolverplugin.h"
#include "editordelayplugin.h"
#include "editorgainplugin.h"
#include "editoriir2highpasfilterplugin.h"
#include "editoriir2lowpasfilterplugin.h"
#include "editorlimiterplugin.h"
#include "editorpanplugin.h"
#include "editorpositioningplugin.h"
#include "mixeractions.h"

rf::MixerView::MixerView(Context* context, const char* name, const char* nameId, const DeserializedData* data)
    : ViewBase(context, name, nameId)
{
    m_actions = Allocator::Allocate<ActionHandler>("MixerActionHandler", std::bind(&MixerView::SetDirtyToTrue, this));
    Refresh();

    if (data)
    {
        const auto GetEditorState = [this](const std::string& name) -> MixGroupEditorState* {
            for (MixGroupEditorState& state : m_editorState)
            {
                if (state.m_mixGroup->GetName() == name)
                {
                    return &state;
                }
            }
            return nullptr;
        };

        const auto editorState = data->m_json["mixer"]["editorState"];
        for (const auto& state : editorState)
        {
            const std::string name = state.value("name", "");
            if (MixGroupEditorState* editorState = GetEditorState(name))
            {
                const int drawOrder = state.value("drawOrder", 0);
                std::vector<bool> editPlugins = state["editPlugins"];
                std::vector<float> colour = state["colour"];

                editorState->m_drawOrder = drawOrder;

                for (int i = 0; i < editPlugins.size(); ++i)
                {
                    editorState->m_editPlugin[i] = editPlugins[i];
                }

                for (int i = 0; i < colour.size(); ++i)
                {
                    editorState->m_colour[i] = colour[i];
                }
            }
        }

        SortEditorState();
    }
}

rf::MixerView::~MixerView()
{
    Allocator::Deallocate<ActionHandler>(&m_actions);

    for (MixGroupEditorState& editorState : m_editorState)
    {
        for (int j = 0; j < RF_MAX_MIX_GROUP_SENDS; ++j)
        {
            delete editorState.m_plugins[j];
            editorState.m_plugins[j] = nullptr;
        }
    }
}

void rf::MixerView::Refresh()
{
    for (MixGroupEditorState& editorState : m_editorState)
    {
        for (int j = 0; j < RF_MAX_MIX_GROUP_SENDS; ++j)
        {
            delete editorState.m_plugins[j];
            editorState.m_plugins[j] = nullptr;
        }
    }
    m_editorState.clear();

    m_mixerSystem = m_context->GetMixerSystem();

    int lastDrawOrder = -1;
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        MixGroup* mixGroup = m_mixerSystem->GetMixGroup(i);

        if (*mixGroup)
        {
            MixGroupEditorState& editorState = m_editorState.emplace_back();
            editorState.m_mixGroup = mixGroup;
            editorState.m_drawOrder = i;

            // Check for sends
            for (int j = 0; j < RF_MAX_MIX_GROUP_SENDS; ++j)
            {
                if (Send* send = mixGroup->GetSend(j))
                {
                    editorState.m_sends[j] = send;
                }
            }

            // Check for plug-ins
            for (int j = 0; j < RF_MAX_MIX_GROUP_PLUGINS; ++j)
            {
                if (const PluginBase* plugin = mixGroup->GetPlugin(j))
                {
                    delete editorState.m_plugins[j];
                    editorState.m_plugins[j] = nullptr;

                    const PluginBase::Type type = plugin->GetType();
                    switch (type)
                    {
                        case PluginBase::Type::ButterworthHighpassFilter:
                        {
                            editorState.m_plugins[j] = new EditorButterworthHighpassFilterPlugin(EditorPlugin::Type::ButterworthHighpassFilter);
                            break;
                        }
                        case PluginBase::Type::ButterworthLowpassFilter:
                        {
                            editorState.m_plugins[j] = new EditorButterworthLowpassFilterPlugin(EditorPlugin::Type::ButterworthLowpassFilter);
                            break;
                        }
                        case PluginBase::Type::Compressor:
                        {
                            editorState.m_plugins[j] = new EditorCompressorPlugin(EditorPlugin::Type::Compressor);
                            break;
                        }
                        case PluginBase::Type::Convolver:
                        {
                            editorState.m_plugins[j] = new EditorConvolverPlugin(EditorPlugin::Type::Convolver);
                            break;
                        }
                        case PluginBase::Type::Delay:
                        {
                            editorState.m_plugins[j] = new EditorDelayPlugin(EditorPlugin::Type::Delay);
                            break;
                        }
                        case PluginBase::Type::Gain:
                        {
                            editorState.m_plugins[j] = new EditorGainPlugin(EditorPlugin::Type::Gain);
                            break;
                        }
                        case PluginBase::Type::IIR2HighpassFilter:
                        {
                            editorState.m_plugins[j] = new EditorIIR2HighpassFilterPlugin(EditorPlugin::Type::IIR2HighpassFilter);
                            break;
                        }
                        case PluginBase::Type::IIR2LowpassFilter:
                        {
                            editorState.m_plugins[j] = new EditorIIR2LowpassFilterPlugin(EditorPlugin::Type::IIR2LowpassFilter);
                            break;
                        }
                        case PluginBase::Type::Limiter:
                        {
                            editorState.m_plugins[j] = new EditorLimiterPlugin(EditorPlugin::Type::Limiter);
                            break;
                        }
                        case PluginBase::Type::Pan:
                        {
                            editorState.m_plugins[j] = new EditorPanPlugin(EditorPlugin::Type::Pan);
                            break;
                        }
                        case PluginBase::Type::Positioning:
                        {
                            editorState.m_plugins[j] = new EditorPositioningPlugin(EditorPlugin::Type::Positioning);
                            break;
                        }
                        case PluginBase::Type::Invalid:
                        default: RF_FAIL("We should not hit this"); break;
                    }

                    uint8_t state[k_pluginStateSize] = {};
                    editorState.m_plugins[j]->InitializeState(state, m_mixerSystem, mixGroup->GetMixGroupHandle(), j);
                    editorState.m_plugins[j]->SetFromState(state, m_mixerSystem, mixGroup->GetMixGroupHandle(), j);
                }
            }

            lastDrawOrder = i;
        }
    }

    MixGroupEditorState& editorState = m_editorState.emplace_back();
    editorState.m_mixGroup = m_mixerSystem->GetMasterMixGroup();
    editorState.m_drawOrder = lastDrawOrder + 1;
}

void rf::MixerView::Update()
{
    VisibiltiyPane();
    ImGui::SameLine();
    MixGroups();
}

rf::ActionHandler* rf::MixerView::GetActionHandler()
{
    return m_actions;
}

void rf::MixerView::Serialize() const
{
    const Version& version = GetVersion();

    nlohmann::ordered_json json;

    json["version"] = {
        {"major", version.m_major},
        {"minor", version.m_minor},
        {"patch", version.m_patch},
    };

    json["mixer"]["editorState"] = m_editorState;

    std::ofstream file("redfisheditor.json");
    file << std::setw(4) << json;
}

rf::MixGroupEditorState& rf::MixerView::GetEditorState(int index)
{
    RF_ASSERT(index >= 0 && index < m_editorState.size(), "Index out of range");
    return m_editorState[index];
}

void rf::MixerView::SortEditorState()
{
    std::sort(m_editorState.begin(), m_editorState.end(), [](const MixGroupEditorState& a, const MixGroupEditorState& b) {
        return a.m_drawOrder < b.m_drawOrder;
    });
}

void rf::MixerView::SetDirtyToTrue()
{
    SetDirty(true);
}

int rf::MixerView::GetEditorStateIndex(const MixGroup* mixGroup)
{
    const int numEntries = static_cast<int>(m_editorState.size());
    for (int i = 0; i < numEntries; ++i)
    {
        if (m_editorState[i].m_mixGroup == mixGroup)
        {
            return i;
        }
    }

    RF_FAIL("Cannot find mix group");
    return -1;
}

bool rf::MixerView::MixGroupComboBox(const char* label, int* inOutIndex, int ignoreIndex)
{
    bool returnValue = false;
    if (ImGui::BeginCombo(label, m_editorState[*inOutIndex].m_mixGroup->GetName()))
    {
        const int numEntries = static_cast<int>(m_editorState.size());
        for (int i = 0; i < numEntries; ++i)
        {
            if (i == ignoreIndex)
            {
                continue;
            }

            const bool isSelected = *inOutIndex == i;
            if (ImGui::Selectable(m_editorState[i].m_mixGroup->GetName(), isSelected))
            {
                *inOutIndex = i;
                returnValue = true;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    return returnValue;
}

void rf::MixerView::VisibiltiyPane()
{
    ImGui::BeginChild("VisibiltiyPane", ImVec2(175, -1), true, ImGuiWindowFlags_HorizontalScrollbar);

    if (ImGui::Button("Create"))
    {
        ImGui::OpenPopup("Create");
    }

    if (m_editorState.size() > 1)
    {
        ImGui::SameLine();

        if (ImGui::Button("Destroy"))
        {
            ImGui::OpenPopup("Destroy");
        }
    }

    if (ImGui::BeginPopupModal("Create", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char s_name[RF_MAX_NAME_SIZE];
        ImGui::Text("Enter mix group name.");

        ImGui::InputText("Name", s_name, RF_MAX_NAME_SIZE);

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            if (m_mixerSystem->CanCreateMixGroup(s_name))
            {
                m_mixerSystem->CreateMixGroup(s_name);
                m_actions->DeleteActions();
                Refresh();
                ImGui::CloseCurrentPopup();
                s_name[0] = '\0';
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            s_name[0] = '\0';
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Destroy", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Delete Mix Group");

        const int masterIndex = GetEditorStateIndex(m_mixerSystem->GetMasterMixGroup());
        static int s_index = 0;
        const bool changed = MixGroupComboBox("Mix Group", &s_index, masterIndex);

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            MixGroup* toDelete = m_mixerSystem->GetMixGroup(GetEditorState(s_index).m_mixGroup->GetMixGroupHandle());
            m_mixerSystem->DestroyMixGroup(&toDelete);
            m_actions->DeleteActions();
            Refresh();
            ImGui::CloseCurrentPopup();
            s_index = 0;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            s_index = 0;
        }
        ImGui::EndPopup();
    }

    const int numEntries = static_cast<int>(m_editorState.size());
    for (int i = 0; i < numEntries; ++i)
    {
        MixGroupEditorState& editorState = m_editorState[i];
        ImGui::PushID(i);
        if (ImGui::ArrowButton("Up", ImGuiDir_Up))
        {
            if (i > 0)
            {
                const int temp = m_editorState[i - 1].m_drawOrder;
                m_actions->DoAction(new ReorderMixGroup(this, i, i - 1, temp));
            }
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("Down", ImGuiDir_Down))
        {
            if (i < numEntries - 1)
            {
                const int temp = m_editorState[i + 1].m_drawOrder;
                m_actions->DoAction(new ReorderMixGroup(this, i, i + 1, temp));
            }
        }
        ImGui::SameLine();
        ImGui::Checkbox("", &editorState.m_draw);
        ImGui::SameLine();
        ImGui::Text(editorState.m_mixGroup->GetName());
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void rf::MixerView::MixGroups()
{
    ImGui::BeginChild("MixGroups", ImVec2(-1, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    int counter = 0;
    for (MixGroupEditorState& editorState : m_editorState)
    {
        if (!editorState.m_draw)
        {
            continue;
        }

        MixGroupStrip(editorState, counter++);
        ImGui::SameLine();
    }
    ImGui::EndChild();
}

void rf::MixerView::MixGroupStrip(MixGroupEditorState& editorState, int index)
{
    ImGui::PushID(index);

    MixGroup* mixGroup = editorState.m_mixGroup;
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild(mixGroup->GetName(), ImVec2(145.0f, -1), true, windowFlags);

    if (ImGui::BeginMenuBar())
    {
        ImGui::Text(mixGroup->GetName());
        ImGui::EndMenuBar();
    }

    Plugins(editorState, index);
    Sends(editorState, index);
    Fader(editorState);
    Output(editorState, index);
    Name(editorState, index);

    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::PopID();
}

void rf::MixerView::Plugins(MixGroupEditorState& editorState, int index)
{
    const MixGroup* mixGroup = editorState.m_mixGroup;

    ImGui::PushID("Plugins");

    ImGui::Separator();
    ImGui::Text("Plug-ins");
    ImGui::Separator();

    static const ImVec2 s_buttonSize = {105.0f, 0.0f};
    int pluginSlot = -1;

    enum PluginAction
    {
        None,
        Edit,
        Create,
        Destroy
    } action = None;

    for (int i = 0; i < RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        ImGui::PushID(i);

        const bool pluginAtSlot = editorState.m_plugins[i];

        if (pluginAtSlot)
        {
            if (ImGui::Button("x"))
            {
                pluginSlot = i;
                action = Destroy;
            }
        }
        else
        {
            if (ImGui::Button("+"))
            {
                pluginSlot = i;
                action = Create;
            }
        }

        ImGui::SameLine();

        if (pluginAtSlot)
        {
            const char* name = editorState.m_plugins[i]->GetName();
            if (ImGui::Button(name, s_buttonSize))
            {
                pluginSlot = i;
                action = Edit;
            }
        }
        else
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            ImGui::Button("None", s_buttonSize);
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        ImGui::PopID();
    }

    static int s_pluginSlot = -1;

    if (pluginSlot >= 0)
    {
        s_pluginSlot = pluginSlot;

        switch (action)
        {
            case None: break;
            case Edit:
            {
                editorState.m_editPlugin[s_pluginSlot] = true;
                break;
            }
            case Create:
            {
                ImGui::OpenPopup("Create Plugin");
                break;
            }
            case Destroy:
            {
                const MixGroupHandle mixGroupHandle = editorState.m_mixGroup->GetMixGroupHandle();
                m_actions->DoAction(new DestroyPluginAction(&m_editorState, index, m_mixerSystem, mixGroup->GetMixGroupHandle(), s_pluginSlot));
                break;
            }
            default: RF_FAIL("Unhandled case"); break;
        }
    }

    if (ImGui::BeginPopupModal("Create Plugin", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Chose which plug-in to create.");

        static_assert(static_cast<int>(EditorPlugin::Type::Version) == 1, "Update this");
        static constexpr int k_numPlugins = 11;

        static EditorPlugin::Type s_types[k_numPlugins] = {
            EditorPlugin::Type::ButterworthHighpassFilter,
            EditorPlugin::Type::ButterworthLowpassFilter,
            EditorPlugin::Type::Compressor,
            EditorPlugin::Type::Convolver,
            EditorPlugin::Type::Delay,
            EditorPlugin::Type::Gain,
            EditorPlugin::Type::IIR2HighpassFilter,
            EditorPlugin::Type::IIR2LowpassFilter,
            EditorPlugin::Type::Limiter,
            EditorPlugin::Type::Pan,
            EditorPlugin::Type::Positioning,
        };

        static const char* s_names[k_numPlugins] = {
            EditorPlugin::GetPluginName(EditorPlugin::Type::ButterworthHighpassFilter),
            EditorPlugin::GetPluginName(EditorPlugin::Type::ButterworthLowpassFilter),
            EditorPlugin::GetPluginName(EditorPlugin::Type::Compressor),
            EditorPlugin::GetPluginName(EditorPlugin::Type::Convolver),
            EditorPlugin::GetPluginName(EditorPlugin::Type::Delay),
            EditorPlugin::GetPluginName(EditorPlugin::Type::Gain),
            EditorPlugin::GetPluginName(EditorPlugin::Type::IIR2HighpassFilter),
            EditorPlugin::GetPluginName(EditorPlugin::Type::IIR2LowpassFilter),
            EditorPlugin::GetPluginName(EditorPlugin::Type::Limiter),
            EditorPlugin::GetPluginName(EditorPlugin::Type::Pan),
            EditorPlugin::GetPluginName(EditorPlugin::Type::Positioning),
        };

        static int s_pluginIndex = 0;
        ImGui::Combo("", &s_pluginIndex, s_names, k_numPlugins);

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            const EditorPlugin::Type type = s_types[s_pluginIndex];
            m_actions->DoAction(new CreatePluginAction(&m_editorState, index, m_mixerSystem, mixGroup->GetMixGroupHandle(), s_pluginSlot, type));
            ImGui::CloseCurrentPopup();
            editorState.m_editPlugin[s_pluginSlot] = true;
            s_pluginIndex = 0;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            s_pluginIndex = 0;
        }
        ImGui::EndPopup();
    }

    for (int i = 0; i < RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        if (editorState.m_editPlugin[i] && editorState.m_plugins[i])
        {
            editorState.m_plugins[i]->Edit(&editorState.m_editPlugin[i], m_mixerSystem, mixGroup->GetMixGroupHandle(), i, m_actions);
        }
    }

    ImGui::PopID();
}

void rf::MixerView::Sends(MixGroupEditorState& editorState, int index)
{
    ImGui::PushID("Sends");
    ImGui::PushID(index);

    ImGui::Separator();
    ImGui::Text("Sends");
    ImGui::Separator();

    static const ImVec2 s_sendButtonSize = {105.0f, 0.0f};
    int sendSlot = -1;

    enum SendAction
    {
        None,
        Edit,
        Create,
        Destroy
    } action = None;

    for (int i = 0; i < RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        ImGui::PushID(i);

        const bool sendAtSlot = editorState.m_sends[i];

        if (sendAtSlot)
        {
            if (ImGui::Button("x"))
            {
                sendSlot = i;
                action = Destroy;
            }
        }
        else
        {
            if (ImGui::Button("+"))
            {
                sendSlot = i;
                action = Create;
            }
        }

        ImGui::SameLine();

        if (sendAtSlot)
        {
            const char* name = m_mixerSystem->GetMixGroup(editorState.m_sends[i]->GetSendToMixGroupHandle())->GetName();
            if (ImGui::Button(name, s_sendButtonSize))
            {
                sendSlot = i;
                action = Edit;
            }
        }
        else
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            ImGui::Button("None", s_sendButtonSize);
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        ImGui::PopID();
    }

    static int s_sendToIndex = -1;
    static int s_sendSlot = -1;

    if (sendSlot >= 0)
    {
        s_sendToIndex = index == 0 ? 1 : 0;
        s_sendSlot = sendSlot;

        switch (action)
        {
            case None: break;
            case Edit:
            {
                ImGui::OpenPopup("Edit Send");
                break;
            }
            case Create:
            {
                ImGui::OpenPopup("Create Send");
                break;
            }
            case Destroy:
            {
                const MixGroupHandle mixGroupHandle = editorState.m_mixGroup->GetMixGroupHandle();
                m_actions->DoAction(new DestroySendAction(&m_editorState, index, m_mixerSystem, mixGroupHandle, s_sendSlot));
                break;
            }
            default: RF_FAIL("Unhandled case"); break;
        }
    }

    if (ImGui::BeginPopupModal("Create Send", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Chose which Mix Group you want to send to.");
        MixGroupComboBox("", &s_sendToIndex, index);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            const MixGroupEditorState& sendState = GetEditorState(s_sendToIndex);
            const MixGroupHandle mixGroupHandle = editorState.m_mixGroup->GetMixGroupHandle();
            const MixGroupHandle sendToHandle = sendState.m_mixGroup->GetMixGroupHandle();
            m_actions->DoAction(new CreateSendAction(&m_editorState, index, m_mixerSystem, mixGroupHandle, sendToHandle, s_sendSlot));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Edit Send", ImGuiWindowFlags_MenuBar))
    {
        const MixGroup* mixGroup = editorState.m_mixGroup;
        Send* send = editorState.m_sends[s_sendSlot];

        if (ImGui::BeginMenuBar())
        {
            const char* sendToName = m_mixerSystem->GetMixGroup(send->GetSendToMixGroupHandle())->GetName();
            ImGui::Text("Edit Send: %s to %s", mixGroup->GetName(), sendToName);
            ImGui::EndMenuBar();
        }

        const float sendDb = send->GetVolumeDb();
        static float s_firstValue;
        static bool s_first = true;
        float value = sendDb;
        const bool active = ImGui::DragFloat("Volume dB", &value, 0.01f, RF_MIN_DECIBELS, RF_MAX_DECIBELS);
        const bool end = ImGui::IsItemDeactivatedAfterEdit();
        send->SetVolumeDb(value);

        if (active && s_first)
        {
            s_firstValue = sendDb;
            s_first = false;
        }

        if (end)
        {
            m_actions->DoAction(new SetSendVolume(m_mixerSystem, mixGroup->GetMixGroupHandle(), s_sendSlot, s_firstValue, value));
            s_first = true;
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();
    ImGui::PopID();
}

void rf::MixerView::Fader(MixGroupEditorState& editorState)
{
    static const ImVec2 k_size = {60.0f, 150.0f};
    static const ImVec2 k_buttonSize = {k_size.x, 25.0f};

    MixGroup* mixGroup = editorState.m_mixGroup;

    ImGui::PushID("Fader");

    ImGui::Separator();
    ImGui::Text("Volume");
    ImGui::Separator();

    static constexpr int k_numChannels = 2;

    // Meter
    float currentDb = -FLT_MAX;
    for (int i = 0; i < k_numChannels; ++i)
    {
        const float amp = mixGroup->GetCurrentAmplitude();
        const float amps[1] = {amp};
        ImGui::PlotHistogram("", amps, IM_ARRAYSIZE(amps), 0, nullptr, 0.0f, 1.0f, ImVec2(k_size.x * 0.5f, k_size.y));
        ImGui::SameLine(0.0f, 0.0f);
        const float db = Functions::AmplitudeToDecibel(amp);
        if (db > editorState.m_peakDb)
        {
            editorState.m_peakDb = db;
        }

        if (db > currentDb)
        {
            currentDb = db;
        }
    }

    ImGui::SameLine();

    // Fader
    const float faderDb = mixGroup->GetVolumeDb();
    static float s_firstValue;
    static bool s_first = true;
    float value = faderDb;
    bool active = ImGui::VSliderFloat("", k_size, &value, RF_MIN_DECIBELS, RF_MAX_DECIBELS, "%.2fdB");
    bool end = ImGui::IsItemDeactivatedAfterEdit();
    mixGroup->SetVolumeDb(value);

    if (ImGui::BeginPopupContextItem("fader context menu"))
    {
        if (ImGui::Button("Set to 0.0dB"))
        {
            active = true;
            end = true;
            value = 0.0f;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetNextItemWidth(-1);
        active = ImGui::DragFloat("##Value", &value, 0.1f, RF_MIN_DECIBELS, RF_MAX_DECIBELS, "%.2fdB") || active;
        end = ImGui::IsItemDeactivatedAfterEdit() || end;

        if (active)
        {
            s_first = true;
        }

        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (active && s_first)
    {
        s_firstValue = faderDb;
        s_first = false;
    }

    if (end)
    {
        m_actions->DoAction(new SetMixGroupVolume(m_mixerSystem, mixGroup->GetMixGroupHandle(), s_firstValue, value));
        s_first = true;
    }

    // Current dB
    {
        char buffer[32];
        sprintf_s(buffer, "%.2fdb", currentDb);
        ImGui::Button(buffer, k_buttonSize);
    }

    ImGui::SameLine();

    // Peak dB
    {
        char buffer[32];
        sprintf_s(buffer, "%.2fdb", editorState.m_peakDb);
        if (ImGui::Button(buffer, k_buttonSize))
        {
            editorState.m_peakDb = -FLT_MAX;
        }
    }

    // Clipping Indicator
    ImVec4 peakColour;
    if (editorState.m_peakDb >= 0.0f)
    {
        peakColour = ImVec4(255.0f, 0.0f, 0.0f, 0.0f);
    }
    else if (editorState.m_peakDb <= 0.0f && editorState.m_peakDb >= -6.0f)
    {
        peakColour = ImVec4(230.0f, 179.0f, 0.0f, 0.0f);
    }
    else
    {
        peakColour = ImVec4(0.0f, 255.0f, 0.0f, 0.0f);
    }

    ImGui::ColorButton("peak", peakColour, ImGuiColorEditFlags_NoTooltip, ImVec2(9.0f + (k_size.x * 2.0f), 10.0f));

    ImGui::PopID();
}

void rf::MixerView::Output(MixGroupEditorState& editorState, int index)
{
    ImGui::PushID("Output");

    ImGui::Separator();
    ImGui::Text("Output");
    ImGui::Separator();

    if (strcmp(editorState.m_mixGroup->GetName(), "Master") == 0)
    {
        if (ImGui::BeginCombo("##MasterOut", "Device"))
        {
            ImGui::EndCombo();
        }
        ImGui::PopID();
        return;
    }

    int outputIndex = GetEditorStateIndex(editorState.m_mixGroup->GetOutputMixGroup());
    const bool changed = MixGroupComboBox("", &outputIndex, index);

    if (changed)
    {
        m_actions->DoAction(new SetMixGroupOutput(
            m_mixerSystem, editorState.m_mixGroup->GetMixGroupHandle(), GetEditorState(outputIndex).m_mixGroup->GetMixGroupHandle()));
    }

    ImGui::PopID();
}

void rf::MixerView::Name(MixGroupEditorState& editorState, int index)
{
    const MixGroup* mixGroup = editorState.m_mixGroup;

    const float* stateColour = editorState.m_colour;
    const ImVec4 color = {stateColour[0], stateColour[1], stateColour[2], 1.0f};

    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);

    static bool s_savedPaletteInit = true;
    static ImVec4 s_savedPalette[32] = {};
    if (s_savedPaletteInit)
    {
        for (int n = 0; n < IM_ARRAYSIZE(s_savedPalette); n++)
        {
            ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f, s_savedPalette[n].x, s_savedPalette[n].y, s_savedPalette[n].z);
            s_savedPalette[n].w = 1.0f;
        }
        s_savedPaletteInit = false;
    }

    ImGui::Separator();
    if (ImGui::Button(mixGroup->GetName(), ImVec2(-1, 20)))
    {
        ImGui::OpenPopup("mypicker");
    }
    if (ImGui::BeginPopup("mypicker"))
    {
        ImGui::BeginGroup();
        for (int n = 0; n < IM_ARRAYSIZE(s_savedPalette); n++)
        {
            ImGui::PushID(n);
            if ((n % 8) != 0)
            {
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
            }

            ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
            if (ImGui::ColorButton("##palette", s_savedPalette[n], palette_button_flags, ImVec2(20, 20)))
            {
                const ImVec4 colour = ImVec4(s_savedPalette[n].x, s_savedPalette[n].y, s_savedPalette[n].z, 1.0f);
                m_actions->DoAction(new SetMixGroupColour(&m_editorState, index, colour.x, colour.y, colour.z));
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }
        ImGui::EndGroup();
        ImGui::EndPopup();
    }
    ImGui::Separator();

    ImGui::PopStyleColor(3);
}
