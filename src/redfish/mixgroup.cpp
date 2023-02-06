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

#include "mixgroup.h"

#include "commandprocessor.h"
#include "functions.h"
#include "mixercommands.h"

rf::MixGroup::MixGroup(Context* context, CommandProcessor* commands, MixerSystem* mixerSystem, const char* name)
    : m_context(context)
    , m_commands(commands)
    , m_mixerSystem(mixerSystem)
    , m_output(m_mixerSystem ? m_mixerSystem->GetMasterMixGroup() : nullptr)
    , m_mixGroupHandle(m_commands ? CreateMixGroupHandle() : MixGroupHandle())
{
    if (name)
    {
        strcpy_s(m_name, name);
    }
    else
    {
        m_name[0] = '\0';
    }
}

rf::MixGroup::operator bool() const
{
    return static_cast<bool>(m_commands);
}

rf::MixGroupHandle rf::MixGroup::GetMixGroupHandle() const
{
    return m_mixGroupHandle;
}

void rf::MixGroup::SetVolumeDb(float volumeDb)
{
    const float oldVolumeDb = GetVolumeDb();
    if (Functions::FloatEquality(oldVolumeDb, volumeDb))
    {
        return;
    }

    const int index = m_mixerSystem->GetMixGroupIndex(m_mixGroupHandle);
    m_mixerSystem->GetMixGroupState(index).m_volumeDb = volumeDb;

    AudioCommand cmd;
    SetMixGroupAmplitudeCommand& data = EncodeAudioCommand<SetMixGroupAmplitudeCommand>(&cmd);
    data.m_mixGroupHandle = m_mixGroupHandle;
    data.m_amplitude = Functions::DecibelToAmplitude(volumeDb);
    m_commands->Add(cmd);
}

float rf::MixGroup::GetVolumeDb() const
{
    return m_mixerSystem->GetMixGroupState(m_mixGroupHandle).m_volumeDb;
}

void rf::MixGroup::SetOutputMixGroup(const MixGroup* mixGroup)
{
    const int index = m_mixerSystem->GetMixGroupIndex(m_mixGroupHandle);
    const MixGroupHandle output = mixGroup->GetMixGroupHandle();
    m_mixerSystem->GetMixGroupState(index).m_outputMixGroupHandle = output;
    const float priority = m_mixerSystem->UpdateMixGroupPriority(index);

    AudioCommand cmd;
    SetMixGroupOutputCommand& data = EncodeAudioCommand<SetMixGroupOutputCommand>(&cmd);
    data.m_mixGroupHandle = m_mixGroupHandle;
    data.m_priority = priority;
    data.m_outputMixGroupHandle = output;
    m_commands->Add(cmd);

    m_output = m_mixerSystem->GetMixGroup(output);
}

rf::Send* rf::MixGroup::CreateSend(const MixGroup* mixGroup, int slot)
{
    if (!m_mixerSystem->CanCreateSend())
    {
        RF_FAIL("Too many sends created in project. Increase RF_MAX_MIX_GROUP_SENDS");
        return nullptr;
    }

    RF_ASSERT(slot >= 0 && slot < RF_MAX_MIX_GROUP_SENDS, "slot out of bounds");

    int sendIndex;
    const MixGroupHandle sendToHandle = mixGroup->GetMixGroupHandle();
    Send* send = m_mixerSystem->CreateSend(sendToHandle, &sendIndex);

    MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
    state.m_sendSlots[slot] = sendIndex;
    const int stateIndex = m_mixerSystem->GetMixGroupIndex(m_mixGroupHandle);
    const float priority = m_mixerSystem->UpdateMixGroupPriority(stateIndex);

    AudioCommand cmd;
    CreateSendCommand& data = EncodeAudioCommand<CreateSendCommand>(&cmd);
    data.m_sendIndex = sendIndex;
    data.m_mixGroupSlot = slot;
    data.m_priority = priority;
    data.m_mixGroupHandle = m_mixGroupHandle;
    data.m_sendToMixGroupHandle = sendToHandle;
    m_commands->Add(cmd);

    return send;
}

rf::Send* rf::MixGroup::GetSend(int slot) const
{
    RF_ASSERT(slot >= 0 && slot < RF_MAX_MIX_GROUP_SENDS, "Index out of bounds");
    const MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
    const int index = state.m_sendSlots[slot];
    return index == -1 ? nullptr : m_mixerSystem->GetSend(state.m_sendSlots[slot]);
}

rf::Send* rf::MixGroup::CreateSend(const MixGroup* mixGroup)
{
    if (!m_mixerSystem->CanCreateSend())
    {
        RF_FAIL("Too many sends created in project. Increase RF_MAX_MIX_GROUP_SENDS");
        return nullptr;
    }

    int mixGroupSlot = -1;
    MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
    for (int i = 0; i < RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        if (state.m_sendSlots[i] == -1)
        {
            mixGroupSlot = i;
            break;
        }
    }

    return CreateSend(mixGroup, mixGroupSlot);
}

void rf::MixGroup::DestroySend(Send** send)
{
    if (!(*send))
    {
        return;
    }

    const int sendIndex = m_mixerSystem->DestroySend(*send);
    int mixGroupSlot = -1;
    MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
    for (int i = 0; i < RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        if (state.m_sendSlots[i] == sendIndex)
        {
            mixGroupSlot = i;
            state.m_sendSlots[i] = -1;
            break;
        }
    }

    const int stateIndex = m_mixerSystem->GetMixGroupIndex(m_mixGroupHandle);
    const float priority = m_mixerSystem->UpdateMixGroupPriority(stateIndex);

    AudioCommand cmd;
    DestroySendCommand& data = EncodeAudioCommand<DestroySendCommand>(&cmd);
    data.m_sendIndex = sendIndex;
    data.m_mixGroupSlot = mixGroupSlot;
    data.m_priority = priority;
    data.m_mixGroupHandle = m_mixGroupHandle;
    m_commands->Add(cmd);

    *send = nullptr;
}

float rf::MixGroup::GetCurrentAmplitude() const
{
    return m_mixerSystem->GetMixGroupState(m_mixGroupHandle).m_peakAmplitude;
}

rf::MixGroup* rf::MixGroup::GetOutputMixGroup()
{
    return m_output;
}

const char* rf::MixGroup::GetName() const
{
    return m_name;
}

rf::PluginBase* rf::MixGroup::GetPlugin(int slot)
{
    if (slot >= 0 && slot < RF_MAX_MIX_GROUP_PLUGINS)
    {
        const MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
        const int index = state.m_pluginSlots[slot];
        if (index == -1)
        {
            return nullptr;
        }
        return m_mixerSystem->GetPlugin(state.m_pluginSlots[slot]);
    }

    RF_FAIL("Slot out of bounds");
    return nullptr;
}