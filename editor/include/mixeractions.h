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

#include <vector>

#include "action.h"
#include "editorplugin.h"

namespace rf
{
static constexpr int k_pluginStateSize = 64;

class MixerSystem;
class MixerView;
class Send;
struct MixGroupEditorState;

class SetMixGroupVolume final : public Action
{
public:
    SetMixGroupVolume(MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, float initialVolumeDb, float setTovolumeDb);

    void Do() override;
    void Undo() override;

private:
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    float m_initialVolumeDb = 0.0f;
    float m_setVolumeDb = 0.0f;
};

class ReorderMixGroup final : public Action
{
public:
    ReorderMixGroup(MixerView* view, int index, int otherIndex, int newDrawOrder);

    void Do() override;
    void Undo() override;

private:
    MixerView* m_view = nullptr;
    int m_index = -1;
    int m_otherIndex = -1;
    int m_newDrawOrder = -1;
    int m_oldDrawOrder = -1;
};

class SetMixGroupOutput final : public Action
{
public:
    SetMixGroupOutput(MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, MixGroupHandle outputHandle);

    void Do() override;
    void Undo() override;

private:
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    MixGroupHandle m_newOutputHandle;
    MixGroupHandle m_oldOutputHandle;
};

class CreateSendAction final : public Action
{
public:
    CreateSendAction(std::vector<MixGroupEditorState>* editorState,
                     int index,
                     MixerSystem* mixerSystem,
                     MixGroupHandle mixGroupHandle,
                     MixGroupHandle sendToHandle,
                     int slot);

    void Do() override;
    void Undo() override;

private:
    std::vector<MixGroupEditorState>* m_editorState = nullptr;
    int m_index = -1;
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    MixGroupHandle m_sendToHandle;
    int m_slot = -1;
};

class DestroySendAction final : public Action
{
public:
    DestroySendAction(std::vector<MixGroupEditorState>* editorState, int index, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot);

    void Do() override;
    void Undo() override;

private:
    std::vector<MixGroupEditorState>* m_editorState = nullptr;
    int m_index = -1;
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    MixGroupHandle m_sendToHandle;
    int m_slot = -1;
    float m_volumeDb = 0.0f;
};

class SetSendVolume final : public Action
{
public:
    SetSendVolume(MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int sendSlot, float initialVolumeDb, float setTovolumeDb);

    void Do() override;
    void Undo() override;

private:
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    int m_sendSlot = -1;
    float m_initialVolumeDb = 0.0f;
    float m_setVolumeDb = 0.0f;
};

class SetMixGroupColour final : public Action
{
public:
    SetMixGroupColour(std::vector<MixGroupEditorState>* editorState, int index, float r, float g, float b);

    void Do() override;
    void Undo() override;

private:
    std::vector<MixGroupEditorState>* m_editorState = nullptr;
    int m_index = -1;
    float m_r = 0.0f;
    float m_g = 0.0f;
    float m_b = 0.0f;
    float m_initialR = 0.0f;
    float m_initialG = 0.0f;
    float m_initialB = 0.0f;
};

class CreatePluginAction final : public Action
{
public:
    CreatePluginAction(std::vector<MixGroupEditorState>* editorState,
                       int index,
                       MixerSystem* mixerSystem,
                       MixGroupHandle mixGroupHandle,
                       int slot,
                       EditorPlugin::Type type);

    void Do() override;
    void Undo() override;

private:
    uint8_t m_state[k_pluginStateSize] = {};
    std::vector<MixGroupEditorState>* m_editorState = nullptr;
    int m_index = -1;
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    int m_slot = -1;
    EditorPlugin::Type m_type = EditorPlugin::Type::Invalid;
};

class DestroyPluginAction final : public Action
{
public:
    DestroyPluginAction(std::vector<MixGroupEditorState>* editorState, int index, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot);

    void Do() override;
    void Undo() override;

private:
    uint8_t m_state[k_pluginStateSize] = {};
    std::vector<MixGroupEditorState>* m_editorState = nullptr;
    int m_index = -1;
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    int m_slot = -1;
    EditorPlugin::Type m_type = EditorPlugin::Type::Invalid;
};

}  // namespace rf