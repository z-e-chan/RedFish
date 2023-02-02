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
class GainPlugin;
class Send;

class MixGroup
{
public:
    MixGroup(Context* context, CommandProcessor* commands, MixerSystem* mixerSystem);
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
    Send* CreateSend(const MixGroup* mixGroup);
    float GetCurrentAmplitude() const;

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

        int pluginIndex;
        PluginBase** base = m_mixerSystem->GetPluginBaseForCreation(&pluginIndex);
        state.m_pluginSlots[mixGroupSlot] = pluginIndex;
        T* gain = Allocator::Allocate<T>(typeid(T).name(), m_context, m_commands, pluginIndex, mixGroupSlot, m_mixGroupHandle);
        *base = gain;
        return gain;
    }

private:
    Context* m_context = nullptr;
    CommandProcessor* m_commands = nullptr;
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
};
}  // namespace rf