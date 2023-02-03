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
#include "identifiers.h"

namespace rf
{
class CommandProcessor;
class Context;
class MixGroup;
class PluginBase;
class Send;
class Stinger;
struct Message;
struct MixGroupState;
struct Sync;

class MixerSystem
{
public:
    MixerSystem(Context* context, CommandProcessor* commands);
    MixerSystem(const MixerSystem&) = delete;
    MixerSystem(MixerSystem&&) = delete;
    MixerSystem& operator=(const MixerSystem&) = delete;
    MixerSystem& operator=(MixerSystem&&) = delete;
    ~MixerSystem();

    MixGroup* CreateMixGroup();
    MixGroup* GetMasterMixGroup() const;
    void FadeMixGroups(const MixGroup** mixGroups, int numMixGroups, float volumeDb, const Sync& sync, const Sync& duration, const Stinger* stinger);
    void FadeMixGroups(const MixGroup** mixGroups, int numMixGroups, float volumeDb, const Sync& sync, const Sync& duration);

private:
    Context* m_context = nullptr;
    CommandProcessor* m_commands = nullptr;
    MixGroupState* m_mixGroupState = nullptr;
    MixGroup* m_masterMixGroup = nullptr;
    MixGroup* m_mixGroups = nullptr;
    Send* m_sends = nullptr;
    PluginBase** m_plugins = nullptr;
    int m_numMixGroupState = 0;

    int GetMixGroupIndex(MixGroupHandle mixGroupHandle) const;
    void CreateMixGroupInternal(MixGroupHandle mixGroupHandle);
    MixGroupState& GetMixGroupState(MixGroupHandle mixGroupHandle);
    const MixGroupState& GetMixGroupState(MixGroupHandle mixGroupHandle) const;
    MixGroupState& GetMixGroupState(int index);
    const MixGroupState& GetMixGroupState(int index) const;
    float UpdateMixGroupPriority(int index);
    bool CanCreateSend() const;
    Send* CreateSend(MixGroupHandle sendToMixGroupHandle, int* outIndex);
    int DestroySend(const Send* send);
    bool CanCreatePlugin() const;
    PluginBase** GetPluginBaseForCreation(int* outIndex);
    PluginBase** GetPluginBaseForDeletion(const PluginBase* plugin, int* outIndex);
    bool ProcessMessages(const Message& message);

    friend class Context;
    friend class MixGroup;
};
}  // namespace rf