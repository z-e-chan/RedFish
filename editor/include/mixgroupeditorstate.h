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
#include <float.h>
#include <redfish/defines.h>

#include <external/nlohmann/json.hpp>

namespace rf
{
class EditorPlugin;
class MixGroup;
class Send;

struct MixGroupEditorState
{
    MixGroup* m_mixGroup = nullptr;
    Send* m_sends[RF_MAX_MIX_GROUP_SENDS] = {};
    EditorPlugin* m_plugins[RF_MAX_MIX_GROUP_PLUGINS] = {};
    bool m_editPlugin[RF_MAX_MIX_GROUP_PLUGINS] = {};
    int m_drawOrder = 0;
    float m_peakDb = -FLT_MAX;
    float m_colour[3] = {0.159999996f, 0.242580697f, 0.800000012f};
    bool m_draw = true;
};

void to_json(nlohmann::ordered_json& json, const MixGroupEditorState& object);
}  // namespace rf