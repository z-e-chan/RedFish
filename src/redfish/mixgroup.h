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
#include "allocator.h"
#include "assert.h"
#include "defines.h"
#include "identifiers.h"
#include "mixersystem.h"
#include "mixgroupstate.h"

namespace rf
{
class CommandProcessor;
class Context;
class Send;

class MixGroup
{
public:
    MixGroup(Context* context, CommandProcessor* commands, MixerSystem* mixerSystem, const char* name);
    MixGroup(const MixGroup&) = delete;
    MixGroup(MixGroup&&) = delete;
    MixGroup& operator=(const MixGroup&) = delete;
    MixGroup& operator=(MixGroup&&) = delete;
    ~MixGroup() = default;

    explicit operator bool() const;

    MixGroupHandle GetMixGroupHandle() const;
    void SetVolumeDb(float volumeDb);
    float GetVolumeDb() const;
    void SetOutputMixGroup(const MixGroup* mixGroup);
    Send* CreateSend(const MixGroup* mixGroup, int slot);
    Send* CreateSend(const MixGroup* mixGroup);
    Send* GetSend(int slot) const;
    void DestroySend(Send** send);
    float GetCurrentAmplitude() const;
    MixGroup* GetOutputMixGroup();
    const char* GetName() const;
    PluginBase* GetPlugin(int slot);

    template <typename T>
    T* CreatePlugin(int slot)
    {
        if (!m_mixerSystem->CanCreatePlugin())
        {
            RF_FAIL("Too many plugins created in project. Increase RF_MAX_MIX_GROUP_PLUGINS");
            return nullptr;
        }

        const int mixGroupSlot = slot;
        MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);

        int pluginIndex;
        PluginBase** base = m_mixerSystem->GetPluginBaseForCreation(&pluginIndex);
        state.m_pluginSlots[mixGroupSlot] = pluginIndex;
        T* plugin = Allocator::Allocate<T>("plugin", m_context, m_commands, m_mixGroupHandle, mixGroupSlot, pluginIndex);
        *base = plugin;
        return plugin;
    }

    template <typename T>
    T* CreatePlugin()
    {
        if (!m_mixerSystem->CanCreatePlugin())
        {
            RF_FAIL("Too many plugins created in project. Increase RF_MAX_MIX_GROUP_PLUGINS");
            return nullptr;
        }

        int mixGroupSlot = -1;
        MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
        for (int i = 0; i < RF_MAX_MIX_GROUP_PLUGINS; ++i)
        {
            if (state.m_pluginSlots[i] == -1)
            {
                mixGroupSlot = i;
                break;
            }
        }

        if (mixGroupSlot == -1)
        {
            RF_FAIL("Too many create on mix group. Increase RF_MAX_MIX_GROUP_PLUGINS");
            return nullptr;
        }

        return CreatePlugin<T>(mixGroupSlot);
    }

    template <typename T>
    void DestroyPlugin(T** plugin)
    {
        if (!(*plugin))
        {
            return;
        }

        int pluginIndex;
        PluginBase** base = m_mixerSystem->GetPluginBaseForDeletion(*plugin, &pluginIndex);
        Allocator::Deallocate<PluginBase>(base);

        MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
        for (int i = 0; i < RF_MAX_MIX_GROUP_PLUGINS; ++i)
        {
            if (state.m_pluginSlots[i] == pluginIndex)
            {
                state.m_pluginSlots[i] = -1;
                break;
            }
        }

        *plugin = nullptr;
    }

    template <typename T>
    T* GetPlugin(int slot)
    {
        if (slot >= 0 && slot < RF_MAX_MIX_GROUP_PLUGINS)
        {
            const MixGroupState& state = m_mixerSystem->GetMixGroupState(m_mixGroupHandle);
            const int index = state.m_pluginSlots[slot];
            return static_cast<T*>(m_mixerSystem->GetPlugin(index));
        }

        RF_FAIL("Slot out of bounds");
        return nullptr;
    }

private:
    char m_name[RF_MAX_NAME_SIZE] = {};
    Context* m_context = nullptr;
    CommandProcessor* m_commands = nullptr;
    MixerSystem* m_mixerSystem = nullptr;
    MixGroup* m_output = nullptr;
    MixGroupHandle m_mixGroupHandle;
};
}  // namespace rf