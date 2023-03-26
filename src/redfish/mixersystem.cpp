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

#include "mixersystem.h"

#include "butterworthhighpassfilterplugin.h"
#include "butterworthlowpassfilterplugin.h"
#include "commandprocessor.h"
#include "compressorplugin.h"
#include "convolverplugin.h"
#include "delayplugin.h"
#include "functions.h"
#include "gainplugin.h"
#include "iir2highpassfilterplugin.h"
#include "iir2lowpassfilterplugin.h"
#include "limiterplugin.h"
#include "message.h"
#include "mixercommands.h"
#include "mixgroup.h"
#include "panplugin.h"
#include "pluginbase.h"
#include "positioningplugin.h"
#include "send.h"
#include "stinger.h"

rf::MixerSystem::MixerSystem(Context* context, CommandProcessor* commands)
    : m_context(context)
    , m_commands(commands)
{
    Allocate();
}

rf::MixerSystem::~MixerSystem()
{
    Free();
}

bool rf::MixerSystem::CanCreateMixGroup(const char* name)
{
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (m_mixGroups[i] && strcmp(name, m_mixGroups[i].GetName()) == 0)
        {
            return false;
        }
    }

    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (!m_mixGroups[i])
        {
            return true;
        }
    }

    return false;
}

rf::MixGroup* rf::MixerSystem::CreateMixGroup(const char* name)
{
    if (!CanCreateMixGroup(name))
    {
        RF_FAIL("Cannot create mix group");
        return nullptr;
    }

    MixGroup* mixGroup = nullptr;
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (!m_mixGroups[i])
        {
            mixGroup = new (m_mixGroups + i) MixGroup(m_context, m_commands, this, name);
            break;
        }
    }

    if (!mixGroup)
    {
        RF_FAIL("Could not create mix group. Increase RF_MAX_MIX_GROUPS");
        return nullptr;
    }

    const MixGroupHandle mixGroupHandle = mixGroup->GetMixGroupHandle();
    CreateMixGroupInternal(mixGroupHandle);
    return mixGroup;
}

void rf::MixerSystem::DestroyMixGroup(MixGroup** mixGroup)
{
    if (!(*mixGroup))
    {
        return;
    }

    MixGroup* group = *mixGroup;
    const MixGroupHandle mixGroupHandle = group->GetMixGroupHandle();

    // Destroy Sends
    {
        const MixGroupState& state = GetMixGroupState(mixGroupHandle);
        for (int i = 0; i < RF_MAX_MIX_GROUP_SENDS; ++i)
        {
            const int sendIndex = state.m_sendSlots[i];
            if (sendIndex < 0)
            {
                continue;
            }

            Send* send = &m_sends[sendIndex];
            group->DestroySend(&send);
        }
    }

    // Destroy Plug-ins
    {
        const MixGroupState& state = GetMixGroupState(mixGroupHandle);
        for (int i = 0; i < RF_MAX_MIX_GROUP_PLUGINS; ++i)
        {
            const int pluginIndex = state.m_pluginSlots[i];
            if (pluginIndex < 0)
            {
                continue;
            }

            PluginBase* plugin = m_plugins[pluginIndex];
            group->DestroyPlugin(&plugin);
        }
    }

    // Destroy State
    int stateIndex = -1;
    for (int i = 0; i < m_numMixGroupState; ++i)
    {
        if (m_mixGroupState[i].m_mixGroupHandle == mixGroupHandle)
        {
            stateIndex = i;
            break;
        }
    }

    RF_ASSERT(stateIndex >= 0, "Expected to find state");

    m_mixGroupState[stateIndex] = m_mixGroupState[m_numMixGroupState - 1];
    --m_numMixGroupState;
    Sort();

    AudioCommand cmd;
    DestroyMixGroupCommand& data = EncodeAudioCommand<DestroyMixGroupCommand>(&cmd);
    data.m_mixGroupIndex = stateIndex;
    m_commands->Add(cmd);

    // Null Out Mix Group
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (m_mixGroups[i].GetMixGroupHandle() == mixGroupHandle)
        {
            new (m_mixGroups + i) MixGroup(nullptr, nullptr, nullptr, nullptr);
            break;
        }
    }

    *mixGroup = nullptr;
}

rf::MixGroup* rf::MixerSystem::GetMixGroup(MixGroupHandle mixGroupHandle)
{
    if (m_masterMixGroup->GetMixGroupHandle() == mixGroupHandle)
    {
        return m_masterMixGroup;
    }

    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (m_mixGroups[i].GetMixGroupHandle() == mixGroupHandle)
        {
            return &m_mixGroups[i];
        }
    }

    RF_FAIL("Could not find mix group");
    return nullptr;
}

const rf::MixGroup* rf::MixerSystem::GetMixGroup(MixGroupHandle mixGroupHandle) const
{
    if (m_masterMixGroup->GetMixGroupHandle() == mixGroupHandle)
    {
        return m_masterMixGroup;
    }

    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (m_mixGroups[i].GetMixGroupHandle() == mixGroupHandle)
        {
            return &m_mixGroups[i];
        }
    }

    RF_FAIL("Could not find mix group");
    return nullptr;
}

rf::MixGroup* rf::MixerSystem::GetMixGroup(const char* name)
{
    if (strcmp(m_masterMixGroup->GetName(), name) == 0)
    {
        return m_masterMixGroup;
    }

    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (strcmp(m_mixGroups[i].GetName(), name) == 0)
        {
            return &m_mixGroups[i];
        }
    }

    RF_FAIL("Could not find mix group");
    return nullptr;
}

rf::MixGroup* rf::MixerSystem::GetMixGroup(int index)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_MIX_GROUPS, "Index out of bounds");
    return &m_mixGroups[index];
}

rf::MixGroup* rf::MixerSystem::GetMasterMixGroup() const
{
    return m_masterMixGroup;
}

void rf::MixerSystem::FadeMixGroups(const MixGroup** mixGroups,
                                    int numMixGroups,
                                    float volumeDb,
                                    const Sync& sync,
                                    const Sync& duration,
                                    const Stinger* stinger)
{
    RF_ASSERT(numMixGroups < RF_MAX_CUE_LAYERS, "Trying to fade too many mix groups");
    RF_ASSERT(sync.m_sync != Sync::Value::Queue, "Queue syncs not supported with mix group fading.");
    RF_ASSERT(duration.m_sync != Sync::Value::Queue, "Queue syncs not supported with mix group fading.");
    AudioCommand cmd;
    FadeMixGroupsCommand& data = EncodeAudioCommand<FadeMixGroupsCommand>(&cmd);
    data.m_sync = sync;
    data.m_duration = duration;
    data.m_stingerIndex = stinger ? stinger->GetIndex() : -1;
    data.m_numMixGroups = numMixGroups;
    data.m_amplitude = Functions::DecibelToAmplitude(volumeDb);
    for (int i = 0; i < numMixGroups; ++i)
    {
        data.m_mixGroupIndices[i] = GetMixGroupIndex(mixGroups[i]->GetMixGroupHandle());
    }
    m_commands->Add(cmd);
}

void rf::MixerSystem::FadeMixGroups(const MixGroup** mixGroups, int numMixGroups, float volumeDb, const Sync& sync, const Sync& duration)
{
    FadeMixGroups(mixGroups, numMixGroups, volumeDb, sync, duration, nullptr);
}

void rf::MixerSystem::Allocate()
{
    Free();
    m_mixGroupState = Allocator::AllocateArray<MixGroupState>("MixGroupState", RF_MAX_MIX_GROUPS);
    m_mixGroups = Allocator::AllocateArray<MixGroup>("MixGroups", RF_MAX_MIX_GROUPS, nullptr, nullptr, nullptr, nullptr);
    m_sends = Allocator::AllocateArray<Send>("Sends", RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS, nullptr, -1, MixGroupHandle());
    m_plugins = Allocator::AllocateArray<PluginBase*>("PluginBase", RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS);
}

void rf::MixerSystem::Free()
{
    Allocator::Deallocate<MixGroup>(&m_masterMixGroup);
    Allocator::DeallocateArray<MixGroupState>(&m_mixGroupState, RF_MAX_MIX_GROUPS);
    Allocator::DeallocateArray<MixGroup>(&m_mixGroups, RF_MAX_MIX_GROUPS);
    Allocator::DeallocateArray<Send>(&m_sends, RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS);

    if (m_plugins)
    {
        for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS; ++i)
        {
            Allocator::Deallocate<PluginBase>(&m_plugins[i]);
        }

        Allocator::DeallocateArray<PluginBase*>(&m_plugins, RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS);
    }

    AudioCommand cmd;
    DestroyAllMixGroupsCommand& data = EncodeAudioCommand<DestroyAllMixGroupsCommand>(&cmd);
    m_commands->Add(cmd);
}

void rf::MixerSystem::CreateMasterMixGroup()
{
    // Create master mix group
    m_masterMixGroup = Allocator::Allocate<MixGroup>("MasterMixGroup", m_context, m_commands, this, "Master");
    CreateMixGroupInternal(m_masterMixGroup->GetMixGroupHandle());
}

int rf::MixerSystem::GetMixGroupIndex(MixGroupHandle mixGroupHandle) const
{
    for (int i = 0; i < m_numMixGroupState; ++i)
    {
        if (m_mixGroupState[i].m_mixGroupHandle == mixGroupHandle)
        {
            return i;
        }
    }

    RF_FAIL("Could not find index");
    return -1;
}

void rf::MixerSystem::CreateMixGroupInternal(MixGroupHandle mixGroupHandle)
{
    float maxPriority = 0.0f;
    for (int i = 0; i < m_numMixGroupState; ++i)
    {
        const float priority = m_mixGroupState[i].m_priority;
        if (priority > maxPriority)
        {
            maxPriority = priority;
        }
    }
    maxPriority += 0.001f;

    const MixGroupHandle masterMixGroupHandle = m_masterMixGroup ? m_masterMixGroup->GetMixGroupHandle() : MixGroupHandle();

    MixGroupState state;
    state.m_mixGroupHandle = mixGroupHandle;
    state.m_outputMixGroupHandle = masterMixGroupHandle;
    state.m_priority = maxPriority;
    state.m_isMaster = mixGroupHandle == masterMixGroupHandle;
    state.m_volumeDb = 0.0f;
    m_mixGroupState[m_numMixGroupState++] = state;

    Sort();

    {
        AudioCommand cmd;
        CreateMixGroupCommand& data = EncodeAudioCommand<CreateMixGroupCommand>(&cmd);
        data.m_mixGroupState = state;
        m_commands->Add(cmd);
    }

    {
        AudioCommand cmd;
        SetMixGroupAmplitudeCommand& data = EncodeAudioCommand<SetMixGroupAmplitudeCommand>(&cmd);
        data.m_mixGroupHandle = mixGroupHandle;
        data.m_amplitude = Functions::DecibelToAmplitude(state.m_volumeDb);
        m_commands->Add(cmd);
    }
}

rf::MixGroupState& rf::MixerSystem::GetMixGroupState(MixGroupHandle mixGroupHandle)
{
    for (int i = 0; i < m_numMixGroupState; ++i)
    {
        if (m_mixGroupState[i].m_mixGroupHandle == mixGroupHandle)
        {
            return m_mixGroupState[i];
        }
    }

    RF_FAIL("Could not find mixgroup state");
    return m_mixGroupState[0];
}

const rf::MixGroupState& rf::MixerSystem::GetMixGroupState(int index) const
{
    return m_mixGroupState[index];
}

rf::MixGroupState& rf::MixerSystem::GetMixGroupState(int index)
{
    return m_mixGroupState[index];
}

const rf::MixGroupState& rf::MixerSystem::GetMixGroupState(MixGroupHandle mixGroupHandle) const
{
    for (int i = 0; i < m_numMixGroupState; ++i)
    {
        if (m_mixGroupState[i].m_mixGroupHandle == mixGroupHandle)
        {
            return m_mixGroupState[i];
        }
    }

    return m_mixGroupState[0];
}

float rf::MixerSystem::UpdateMixGroupPriority(int index)
{
    // Ensures that the returned priority is larger than all of the sent-to mix groups and the output.
    float priority = -1.0f;

    for (int i = 0; i < RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        const int sendIndex = m_mixGroupState[index].m_sendSlots[i];
        if (sendIndex == -1)
        {
            continue;
        }

        const MixGroupHandle sendId = m_sends[sendIndex].GetSendToMixGroupHandle();
        const int sendToIndex = GetMixGroupIndex(sendId);
        const float sendPriority = m_mixGroupState[sendToIndex].m_priority;
        priority = std::max(sendPriority, priority) + 0.001f;
    }

    const int outputIndex = GetMixGroupIndex(m_mixGroupState[index].m_outputMixGroupHandle);
    const float outputPriority = m_mixGroupState[outputIndex].m_priority;
    priority = std::max(outputPriority, priority) + 0.001f;

    m_mixGroupState[index].m_priority = priority;

    Sort();

    return priority;
}

bool rf::MixerSystem::CanCreateSend() const
{
    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        if (!m_sends[i])
        {
            return true;
        }
    }

    return false;
}

rf::Send* rf::MixerSystem::CreateSend(MixGroupHandle sendToMixGroupHandle, int* outIndex)
{
    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        if (!m_sends[i])
        {
            Send* send = new (m_sends + i) Send(m_commands, i, sendToMixGroupHandle);
            *outIndex = i;
            return send;
        }
    }

    RF_FAIL("Cannot create send. Increase RF_MAX_MIX_GROUP_SENDS");
    return nullptr;
}

rf::Send* rf::MixerSystem::GetSend(int index)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS, "Index out of bounds");
    return m_sends[index] ? &m_sends[index] : nullptr;
}

int rf::MixerSystem::DestroySend(const Send* send)
{
    const SendHandle sendHandle = send->GetSendHandle();
    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        if (m_sends[i].GetSendHandle() == sendHandle)
        {
            new (m_sends + i) Send(nullptr, -1, MixGroupHandle());
            return i;
        }
    }

    RF_FAIL("Cannot delete send.");
    return -1;
}

bool rf::MixerSystem::CanCreatePlugin() const
{
    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        if (!m_plugins[i])
        {
            return true;
        }
    }

    return false;
}

rf::PluginBase** rf::MixerSystem::GetPluginBaseForCreation(int* outIndex)
{
    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        if (!m_plugins[i])
        {
            *outIndex = i;
            return &m_plugins[*outIndex];
        }
    }

    RF_FAIL("Cannot create plugin. Increase RF_MAX_MIX_GROUP_PLUGINS");
    return nullptr;
}

rf::PluginBase** rf::MixerSystem::GetPluginBaseForDeletion(const PluginBase* plugin, int* outIndex)
{
    const PluginHandle pluginHandle = plugin->GetPluginHandle();
    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        if (m_plugins[i] && m_plugins[i]->GetPluginHandle() == pluginHandle)
        {
            *outIndex = i;
            return &m_plugins[*outIndex];
        }
    }

    RF_FAIL("Cannot delete plugin.");
    return nullptr;
}

rf::PluginBase* rf::MixerSystem::GetPlugin(int pluginIndex)
{
    RF_ASSERT(pluginIndex >= 0 && pluginIndex < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS, "Index out of bounds");
    return m_plugins[pluginIndex];
}

const rf::PluginBase* rf::MixerSystem::GetPlugin(int pluginIndex) const
{
    RF_ASSERT(pluginIndex >= 0 && pluginIndex < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS, "Index out of bounds");
    return m_plugins[pluginIndex];
}

void rf::MixerSystem::Sort()
{
    std::sort(m_mixGroupState, m_mixGroupState + m_numMixGroupState, [](const MixGroupState& a, const MixGroupState& b) {
        return a.m_priority > b.m_priority;
    });
}

bool rf::MixerSystem::ProcessMessages(const Message& message)
{
    switch (message.m_type)
    {
        case MessageType::MixGroupPeakAmplitude:
        {
            const Message::MixGroupPeakAmplitudeData& data = *message.GetMixGroupPeakAmplitudeData();
            MixGroupState& mixGroupState = GetMixGroupState(data.m_mixGroupIndex);
            mixGroupState.m_peakAmplitude = data.m_amplitude;
            return true;
        }
        case MessageType::MixGroupFadeComplete:
        {
            const Message::MixGroupFadeCompleteData& data = *message.GetMixGroupFadeCompleteData();
            data.m_amplitude;
            data.m_mixGroupHandle;
            return true;
        }
        default: return false;
    }
}

void rf::to_json(nlohmann::ordered_json& json, const MixerSystem& object)
{
    const auto GetName = [&object](MixGroupHandle mixGroupHandle) -> const char* { return object.GetMixGroup(mixGroupHandle)->GetName(); };

    nlohmann::ordered_json j;

    j["state"] = {};
    for (int i = 0; i < object.m_numMixGroupState; ++i)
    {
        nlohmann::ordered_json stateJson = object.m_mixGroupState[i];
        stateJson["name"] = GetName(object.m_mixGroupState[i].m_mixGroupHandle);
        stateJson["output"] = GetName(object.m_mixGroupState[i].m_outputMixGroupHandle);
        j["state"].emplace_back(stateJson);
    }

    j["sends"] = {};
    for (int i = 0; i < object.m_numMixGroupState; ++i)
    {
        for (int k = 0; k < RF_MAX_MIX_GROUP_SENDS; ++k)
        {
            const int sendIndex = object.m_mixGroupState[i].m_sendSlots[k];
            if (sendIndex == -1)
            {
                continue;
            }

            const Send* send = &object.m_sends[sendIndex];
            const char* sendFromName = GetName(object.m_mixGroupState[i].m_mixGroupHandle);
            const char* sendToName = GetName(send->GetSendToMixGroupHandle());

            nlohmann::ordered_json sendJson;
            sendJson["fromMixGroup"] = sendFromName;
            sendJson["toMixGroup"] = sendToName;
            sendJson["volumeDb"] = send->GetVolumeDb();
            sendJson["slot"] = k;
            j["sends"].emplace_back(sendJson);
        }
    }

    j["plugins"] = {};
    for (int i = 0; i < object.m_numMixGroupState; ++i)
    {
        for (int k = 0; k < RF_MAX_MIX_GROUP_PLUGINS; ++k)
        {
            const int pluginIndex = object.m_mixGroupState[i].m_pluginSlots[k];
            if (pluginIndex == -1)
            {
                continue;
            }

            const char* mixGroupName = GetName(object.m_mixGroupState[i].m_mixGroupHandle);
            const PluginBase* plugin = object.GetPlugin(pluginIndex);

            nlohmann::ordered_json pluginJson;
            pluginJson["mixGroup"] = mixGroupName;
            pluginJson["slot"] = k;
            pluginJson["pluginType"] = plugin->GetType();

            nlohmann::ordered_json pluginDataJson;
            pluginDataJson["bypass"] = plugin->GetBypass();
            plugin->ToJson(pluginDataJson);
            pluginJson["pluginData"] = pluginDataJson;
            j["plugins"].emplace_back(pluginJson);
        }
    }

    json = j;
}

void rf::from_json(const nlohmann::ordered_json& json, MixerSystem& object)
{
    object.Free();
    object.Allocate();

    const auto& stateArray = json["state"];
    const int numState = static_cast<int>(stateArray.size());

    // 1. Make sure that master is created first.
    for (int i = 0; i < numState; ++i)
    {
        const auto& data = stateArray.at(i);
        const std::string name = data["name"];
        const float volumeDb = data["volumeDb"];
        const bool isMaster = data["isMaster"];

        if (!isMaster)
        {
            continue;
        }

        object.CreateMasterMixGroup();
        object.m_masterMixGroup->SetVolumeDb(volumeDb);
        break;
    }

    // 2. Create all mix groups.
    for (int i = 0; i < numState; ++i)
    {
        const auto& data = stateArray.at(i);
        const std::string name = data["name"];
        const float volumeDb = data["volumeDb"];
        const bool isMaster = data["isMaster"];

        if (isMaster)
        {
            continue;
        }

        MixGroup* mixGroup = object.CreateMixGroup(name.c_str());
        mixGroup->SetVolumeDb(volumeDb);
    }

    // 3. Assign Outputs
    for (int i = 0; i < numState; ++i)
    {
        const auto& data = stateArray.at(i);
        const std::string name = data["name"];
        const std::string outputName = data["output"];
        const bool isMaster = data["isMaster"];
        if (isMaster)
        {
            continue;
        }

        MixGroup* mixGroup = object.GetMixGroup(name.c_str());
        MixGroup* output = object.GetMixGroup(outputName.c_str());
        mixGroup->SetOutputMixGroup(output);
    }

    // 4. Create sends
    {
        const auto& sendArray = json["sends"];
        const int numSends = static_cast<int>(sendArray.size());

        for (int i = 0; i < numSends; ++i)
        {
            const auto& data = sendArray.at(i);
            const std::string fromName = data["fromMixGroup"];
            const std::string toName = data["toMixGroup"];
            const float volumeDb = data["volumeDb"];
            const int slot = data["slot"];

            MixGroup* from = object.GetMixGroup(fromName.c_str());
            MixGroup* to = object.GetMixGroup(toName.c_str());
            Send* send = from->CreateSend(to, slot);
            send->SetVolumeDb(volumeDb);
        }
    }

    // 5. Create Plugin-ins
    {
        const auto& pluginArray = json["plugins"];
        const int numPlugins = static_cast<int>(pluginArray.size());

        for (int i = 0; i < numPlugins; ++i)
        {
            const auto& data = pluginArray.at(i);
            const std::string mixGroupName = data["mixGroup"];
            const int slot = data["slot"];
            const int type = data["pluginType"];
            const auto pluginData = data["pluginData"];

            MixGroup* mixGroup = object.GetMixGroup(mixGroupName.c_str());
            const PluginBase::Type pluginType = static_cast<PluginBase::Type>(type);
            PluginBase* plugin = nullptr;

            static_assert(static_cast<int>(PluginBase::Type::Version) == 1, "Update switch");

            switch (pluginType)
            {
                case PluginBase::Type::ButterworthHighpassFilter: plugin = mixGroup->CreatePlugin<ButterworthHighpassFilterPlugin>(slot); break;
                case PluginBase::Type::ButterworthLowpassFilter: plugin = mixGroup->CreatePlugin<ButterworthLowpassFilterPlugin>(slot); break;
                case PluginBase::Type::Compressor: plugin = mixGroup->CreatePlugin<CompressorPlugin>(slot); break;
                case PluginBase::Type::Convolver: plugin = mixGroup->CreatePlugin<ConvolverPlugin>(slot); break;
                case PluginBase::Type::Delay: plugin = mixGroup->CreatePlugin<DelayPlugin>(slot); break;
                case PluginBase::Type::Gain: plugin = mixGroup->CreatePlugin<GainPlugin>(slot); break;
                case PluginBase::Type::IIR2HighpassFilter: plugin = mixGroup->CreatePlugin<IIR2HighpassFilterPlugin>(slot); break;
                case PluginBase::Type::IIR2LowpassFilter: plugin = mixGroup->CreatePlugin<IIR2LowpassFilterPlugin>(slot); break;
                case PluginBase::Type::Limiter: plugin = mixGroup->CreatePlugin<LimiterPlugin>(slot); break;
                case PluginBase::Type::Pan: plugin = mixGroup->CreatePlugin<PanPlugin>(slot); break;
                case PluginBase::Type::Positioning: plugin = mixGroup->CreatePlugin<PositioningPlugin>(slot); break;
                case PluginBase::Type::Invalid:
                default: RF_FAIL("Could not deserialize plug-in"); break;
            }

            if (plugin)
            {
                plugin->FromJson(pluginData);
                plugin->SetBypass(pluginData["bypass"]);
            }
        }
    }
}
