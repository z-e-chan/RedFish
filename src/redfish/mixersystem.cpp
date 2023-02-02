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

#include "commandprocessor.h"
#include "functions.h"
#include "message.h"
#include "mixercommands.h"
#include "mixgroup.h"
#include "pluginbase.h"
#include "send.h"
#include "stinger.h"

rf::MixerSystem::MixerSystem(Context* context, CommandProcessor* commands)
    : m_context(context)
    , m_commands(commands)
{
    m_mixGroupState = Allocator::AllocateArray<MixGroupState>("MixGroupState", RF_MAX_MIX_GROUPS);
    m_mixGroups = Allocator::AllocateArray<MixGroup>("MixGroups", RF_MAX_MIX_GROUPS, nullptr, nullptr, nullptr);
    m_sends = Allocator::AllocateArray<Send>("Sends", RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS, nullptr, -1, MixGroupHandle());
    m_plugins = Allocator::AllocateArray<PluginBase*>("PluginBase", RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS);

    // Create master mix group
    m_masterMixGroup = Allocator::Allocate<MixGroup>("MasterMixGroup", m_context, m_commands, this);
    CreateMixGroupInternal(m_masterMixGroup->GetMixGroupHandle());
}

rf::MixerSystem::~MixerSystem()
{
    Allocator::Deallocate<MixGroup>(&m_masterMixGroup);
    Allocator::DeallocateArray<MixGroupState>(&m_mixGroupState, RF_MAX_MIX_GROUPS);
    Allocator::DeallocateArray<MixGroup>(&m_mixGroups, RF_MAX_MIX_GROUPS);
    Allocator::DeallocateArray<Send>(&m_sends, RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_SENDS);

    for (int i = 0; i < RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        Allocator::Deallocate<PluginBase>(&m_plugins[i]);
    }

    Allocator::DeallocateArray<PluginBase*>(&m_plugins, RF_MAX_MIX_GROUPS * RF_MAX_MIX_GROUP_PLUGINS);
}

rf::MixGroup* rf::MixerSystem::CreateMixGroup()
{
    MixGroup* mixGroup = nullptr;
    for (int i = 0; i < RF_MAX_MIX_GROUPS; ++i)
    {
        if (!m_mixGroups[i])
        {
            mixGroup = new (m_mixGroups + i) MixGroup(m_context, m_commands, this);
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
    m_mixGroupState[m_numMixGroupState++] = state;

    std::sort(m_mixGroupState, m_mixGroupState + m_numMixGroupState, [](const MixGroupState& a, const MixGroupState& b) {
        return a.m_priority > b.m_priority;
    });

    AudioCommand cmd;
    CreateMixGroupCommand& data = EncodeAudioCommand<CreateMixGroupCommand>(&cmd);
    data.m_mixGroupState = state;
    m_commands->Add(cmd);
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

    std::sort(m_mixGroupState, m_mixGroupState + m_numMixGroupState, [](const MixGroupState& a, const MixGroupState& b) {
        return a.m_priority > b.m_priority;
    });

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