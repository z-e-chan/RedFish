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
#include "editorplugin.h"
#include "mixeractions.h"

namespace rf
{
class EditorButterworthHighpassFilterPlugin final : public EditorPlugin
{
public:
    EditorButterworthHighpassFilterPlugin(EditorPlugin::Type type);

    void Edit(bool* open, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, ActionHandler* actions) override;
    void InitializeState(void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot) override;
    void SetFromState(const void* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot) override;

    struct State final : public StateBase
    {
        int m_order = 0;
        float m_cutoff = 0.0f;
    };
    static_assert(sizeof(State) < k_pluginStateSize, "struct too big");
};
}  // namespace rf