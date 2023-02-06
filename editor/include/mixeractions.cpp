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

#include "mixeractions.h"

#include <redfish/butterworthhighpassfilterplugin.h>
#include <redfish/butterworthlowpassfilterplugin.h>
#include <redfish/compressorplugin.h>
#include <redfish/convolverplugin.h>
#include <redfish/delayplugin.h>
#include <redfish/gainplugin.h>
#include <redfish/iir2highpassfilterplugin.h>
#include <redfish/iir2lowpassfilterplugin.h>
#include <redfish/limiterplugin.h>
#include <redfish/mixgroup.h>
#include <redfish/panplugin.h>
#include <redfish/positioningplugin.h>
#include <redfish/send.h>

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
#include "mixerview.h"
#include "mixgroupeditorstate.h"

rf::SetMixGroupVolume::SetMixGroupVolume(MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, float initialVolumeDb, float setTovolumeDb)
    : m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_initialVolumeDb(initialVolumeDb)
    , m_setVolumeDb(setTovolumeDb)
{
    const MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    sprintf_s(m_name, "Set '%s' Volume To %.2fdB", mixGroup->GetName(), m_setVolumeDb);
}

void rf::SetMixGroupVolume::Do()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    mixGroup->SetVolumeDb(m_setVolumeDb);
}

void rf::SetMixGroupVolume::Undo()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    mixGroup->SetVolumeDb(m_initialVolumeDb);
}

rf::ReorderMixGroup::ReorderMixGroup(MixerView* view, int index, int otherIndex, int newDrawOrder)
    : m_view(view)
    , m_index(index)
    , m_otherIndex(otherIndex)
    , m_newDrawOrder(newDrawOrder)
    , m_oldDrawOrder(view->GetEditorState(m_index).m_drawOrder)
{
    strcpy_s(m_name, "Reorder Mix Groups");
}

void rf::ReorderMixGroup::Do()
{
    m_view->GetEditorState(m_index).m_drawOrder = m_newDrawOrder;
    m_view->GetEditorState(m_otherIndex).m_drawOrder = m_oldDrawOrder;
    m_view->SortEditorState();
}

void rf::ReorderMixGroup::Undo()
{
    m_view->GetEditorState(m_index).m_drawOrder = m_newDrawOrder;
    m_view->GetEditorState(m_otherIndex).m_drawOrder = m_oldDrawOrder;
    m_view->SortEditorState();
}

rf::SetMixGroupOutput::SetMixGroupOutput(MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, MixGroupHandle outputHandle)
    : m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_newOutputHandle(outputHandle)
    , m_oldOutputHandle(m_mixerSystem->GetMixGroup(m_mixGroupHandle)->GetOutputMixGroup()->GetMixGroupHandle())
{
    const MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    const MixGroup* newOutputMixGroup = m_mixerSystem->GetMixGroup(m_newOutputHandle);
    sprintf_s(m_name, "Set '%s' Output To '%s'", mixGroup->GetName(), newOutputMixGroup->GetName());
}

void rf::SetMixGroupOutput::Do()
{
    m_mixerSystem->GetMixGroup(m_mixGroupHandle)->SetOutputMixGroup(m_mixerSystem->GetMixGroup(m_newOutputHandle));
}

void rf::SetMixGroupOutput::Undo()
{
    m_mixerSystem->GetMixGroup(m_mixGroupHandle)->SetOutputMixGroup(m_mixerSystem->GetMixGroup(m_oldOutputHandle));
}

rf::CreateSendAction::CreateSendAction(std::vector<MixGroupEditorState>* editorState,
                                       int index,
                                       MixerSystem* mixerSystem,
                                       MixGroupHandle mixGroupHandle,
                                       MixGroupHandle sendToHandle,
                                       int slot)
    : m_editorState(editorState)
    , m_index(index)
    , m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_sendToHandle(sendToHandle)
    , m_slot(slot)
{
    const MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    const MixGroup* sendToMixGroup = m_mixerSystem->GetMixGroup(m_sendToHandle);
    sprintf_s(m_name, "Create Send: '%s' to '%s'", mixGroup->GetName(), sendToMixGroup->GetName());
}

void rf::CreateSendAction::Do()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    MixGroup* sendToMixGroup = m_mixerSystem->GetMixGroup(m_sendToHandle);
    rf::Send* send = mixGroup->CreateSend(sendToMixGroup, m_slot);
    (*m_editorState)[m_index].m_sends[m_slot] = send;
}

void rf::CreateSendAction::Undo()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    rf::Send* send = mixGroup->GetSend(m_slot);
    mixGroup->DestroySend(&send);
    (*m_editorState)[m_index].m_sends[m_slot] = nullptr;
}

rf::DestroySendAction::DestroySendAction(std::vector<MixGroupEditorState>* editorState,
                                         int index,
                                         MixerSystem* mixerSystem,
                                         MixGroupHandle mixGroupHandle,
                                         int slot)
    : m_editorState(editorState)
    , m_index(index)
    , m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_slot(slot)
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    Send* send = mixGroup->GetSend(m_slot);
    m_sendToHandle = send->GetSendToMixGroupHandle();
    const MixGroup* sendToMixGroup = m_mixerSystem->GetMixGroup(m_sendToHandle);
    m_volumeDb = send->GetVolumeDb();
    sprintf_s(m_name, "Destroy Send: '%s' to '%s'", mixGroup->GetName(), sendToMixGroup->GetName());
}

void rf::DestroySendAction::Do()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    Send* send = mixGroup->GetSend(m_slot);
    mixGroup->DestroySend(&send);
    (*m_editorState)[m_index].m_sends[m_slot] = nullptr;
}

void rf::DestroySendAction::Undo()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    MixGroup* sendToMixGroup = m_mixerSystem->GetMixGroup(m_sendToHandle);
    Send* send = mixGroup->CreateSend(sendToMixGroup, m_slot);
    send->SetVolumeDb(m_volumeDb);
    (*m_editorState)[m_index].m_sends[m_slot] = send;
}

rf::SetSendVolume::SetSendVolume(MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int sendSlot, float initialVolumeDb, float setTovolumeDb)
    : m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_sendSlot(sendSlot)
    , m_initialVolumeDb(initialVolumeDb)
    , m_setVolumeDb(setTovolumeDb)
{
    const MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    const Send* send = mixGroup->GetSend(m_sendSlot);
    const MixGroup* sendToMixGroup = m_mixerSystem->GetMixGroup(send->GetSendToMixGroupHandle());
    sprintf_s(m_name, "Set Send '%s' to '%s' Volume To %.2fdB", mixGroup->GetName(), sendToMixGroup->GetName(), m_setVolumeDb);
}

void rf::SetSendVolume::Do()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    Send* send = mixGroup->GetSend(m_sendSlot);
    send->SetVolumeDb(m_setVolumeDb);
}

void rf::SetSendVolume::Undo()
{
    MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    Send* send = mixGroup->GetSend(m_sendSlot);
    send->SetVolumeDb(m_initialVolumeDb);
}

rf::SetMixGroupColour::SetMixGroupColour(std::vector<MixGroupEditorState>* editorState, int index, float r, float g, float b)
    : m_editorState(editorState)
    , m_index(index)
    , m_r(r)
    , m_g(g)
    , m_b(b)
{
    const MixGroupEditorState& state = (*m_editorState)[m_index];
    m_initialR = state.m_colour[0];
    m_initialG = state.m_colour[1];
    m_initialB = state.m_colour[2];
    const MixGroup* mixGroup = state.m_mixGroup;
    sprintf_s(m_name, "Set '%s' Colour", mixGroup->GetName());
}

void rf::SetMixGroupColour::Do()
{
    MixGroupEditorState& state = (*m_editorState)[m_index];
    state.m_colour[0] = m_r;
    state.m_colour[1] = m_g;
    state.m_colour[2] = m_b;
}

void rf::SetMixGroupColour::Undo()
{
    MixGroupEditorState& state = (*m_editorState)[m_index];
    state.m_colour[0] = m_initialR;
    state.m_colour[1] = m_initialG;
    state.m_colour[2] = m_initialB;
}

static void DestroyPlugin(std::vector<rf::MixGroupEditorState>* editorState,
                          rf::MixerSystem* mixerSystem,
                          rf::MixGroupHandle mixGroupHandle,
                          int index,
                          int slot,
                          uint8_t* state)
{
    rf::MixGroup* mixGroup = mixerSystem->GetMixGroup(mixGroupHandle);
    rf::EditorPlugin* editorPlugin = (*editorState)[index].m_plugins[slot];
    rf::PluginBase* base = editorPlugin->m_plugin;

#define RF_DELETE_PLUGIN(type)                                                   \
    case rf::EditorPlugin::Type::##type##:                                       \
    {                                                                            \
        editorPlugin->InitializeState(state, mixerSystem, mixGroupHandle, slot); \
        rf::##type##Plugin* plug = static_cast<rf::##type##Plugin*>(base);       \
        mixGroup->DestroyPlugin<rf::##type##Plugin>(&plug);                      \
        delete editorPlugin;                                                     \
        (*editorState)[index].m_plugins[slot] = nullptr;                         \
        break;                                                                   \
    }

    static_assert(static_cast<int>(rf::EditorPlugin::Type::Version) == 1, "Update this");

    switch (editorPlugin->m_type)
    {
        RF_DELETE_PLUGIN(ButterworthHighpassFilter)
        RF_DELETE_PLUGIN(ButterworthLowpassFilter)
        RF_DELETE_PLUGIN(Compressor)
        RF_DELETE_PLUGIN(Convolver)
        RF_DELETE_PLUGIN(Delay)
        RF_DELETE_PLUGIN(Gain)
        RF_DELETE_PLUGIN(IIR2HighpassFilter)
        RF_DELETE_PLUGIN(IIR2LowpassFilter)
        RF_DELETE_PLUGIN(Limiter)
        RF_DELETE_PLUGIN(Pan)
        RF_DELETE_PLUGIN(Positioning)
        default: RF_FAIL("Unhandled case"); break;
    }

#undef RF_DELETE_PLUGIN
}

static rf::PluginBase* CreatePlugin(std::vector<rf::MixGroupEditorState>* editorState,
                                    rf::MixerSystem* mixerSystem,
                                    rf::MixGroupHandle mixGroupHandle,
                                    int index,
                                    int slot,
                                    rf::EditorPlugin::Type type,
                                    uint8_t* state)
{
    rf::MixGroup* mixGroup = mixerSystem->GetMixGroup(mixGroupHandle);
    rf::EditorPlugin** editorPlugin = &(*editorState)[index].m_plugins[slot];

#define RF_CREATE(pluginType)                                                    \
    {                                                                            \
        rf::Editor##pluginType* newPlugin = new rf::Editor##pluginType##(type);  \
        newPlugin->m_plugin = mixGroup->CreatePlugin<rf::pluginType>(slot);      \
        *editorPlugin = newPlugin;                                               \
        (*editorPlugin)->SetFromState(state, mixerSystem, mixGroupHandle, slot); \
        return newPlugin->m_plugin;                                              \
    }

    static_assert(static_cast<int>(rf::EditorPlugin::Type::Version) == 1, "Update this");

    switch (type)
    {
        case rf::EditorPlugin::Type::ButterworthHighpassFilter: RF_CREATE(ButterworthHighpassFilterPlugin);
        case rf::EditorPlugin::Type::ButterworthLowpassFilter: RF_CREATE(ButterworthLowpassFilterPlugin);
        case rf::EditorPlugin::Type::Compressor: RF_CREATE(CompressorPlugin);
        case rf::EditorPlugin::Type::Convolver: RF_CREATE(ConvolverPlugin);
        case rf::EditorPlugin::Type::Delay: RF_CREATE(DelayPlugin);
        case rf::EditorPlugin::Type::Gain: RF_CREATE(GainPlugin);
        case rf::EditorPlugin::Type::IIR2HighpassFilter: RF_CREATE(IIR2HighpassFilterPlugin);
        case rf::EditorPlugin::Type::IIR2LowpassFilter: RF_CREATE(IIR2LowpassFilterPlugin);
        case rf::EditorPlugin::Type::Limiter: RF_CREATE(LimiterPlugin);
        case rf::EditorPlugin::Type::Pan: RF_CREATE(PanPlugin);
        case rf::EditorPlugin::Type::Positioning: RF_CREATE(PositioningPlugin);
        default: RF_FAIL("Unhandled case"); break;
    }

#undef RF_CREATE

    RF_FAIL("Could not create plug-in");
    return nullptr;
}

rf::CreatePluginAction::CreatePluginAction(std::vector<MixGroupEditorState>* editorState,
                                           int index,
                                           MixerSystem* mixerSystem,
                                           MixGroupHandle mixGroupHandle,
                                           int slot,
                                           EditorPlugin::Type type)
    : m_editorState(editorState)
    , m_index(index)
    , m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_slot(slot)
    , m_type(type)
{
    const MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    sprintf_s(m_name, "Create '%s:%i:%s'", mixGroup->GetName(), m_slot, EditorPlugin::GetPluginName(m_type));
}

void rf::CreatePluginAction::Do()
{
    CreatePlugin(m_editorState, m_mixerSystem, m_mixGroupHandle, m_index, m_slot, m_type, nullptr);
}

void rf::CreatePluginAction::Undo()
{
    DestroyPlugin(m_editorState, m_mixerSystem, m_mixGroupHandle, m_index, m_slot, nullptr);
}

rf::DestroyPluginAction::DestroyPluginAction(std::vector<MixGroupEditorState>* editorState,
                                             int index,
                                             MixerSystem* mixerSystem,
                                             MixGroupHandle mixGroupHandle,
                                             int slot)
    : m_editorState(editorState)
    , m_index(index)
    , m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_slot(slot)
{
    const rf::EditorPlugin* editorPlugin = (*editorState)[index].m_plugins[slot];
    const MixGroup* mixGroup = m_mixerSystem->GetMixGroup(m_mixGroupHandle);
    m_type = editorPlugin->m_type;
    sprintf_s(m_name, "Destroy '%s:%i:%s'", mixGroup->GetName(), m_slot, editorPlugin->GetName());
}

void rf::DestroyPluginAction::Do()
{
    DestroyPlugin(m_editorState, m_mixerSystem, m_mixGroupHandle, m_index, m_slot, m_state);
}

void rf::DestroyPluginAction::Undo()
{
    CreatePlugin(m_editorState, m_mixerSystem, m_mixGroupHandle, m_index, m_slot, m_type, m_state);
}
