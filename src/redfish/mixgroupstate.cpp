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

#include "mixgroupstate.h"

rf::MixGroupState::MixGroupState()
{
    for (int i = 0; i < RF_MAX_MIX_GROUP_SENDS; ++i)
    {
        m_sendSlots[i] = -1;
    }

    for (int i = 0; i < RF_MAX_MIX_GROUP_PLUGINS; ++i)
    {
        m_pluginSlots[i] = -1;
    }
}

void rf::to_json(nlohmann::ordered_json& json, const MixGroupState& object)
{
    nlohmann::ordered_json j;
    j["volumeDb"] = object.m_volumeDb;
    j["isMaster"] = object.m_isMaster;
    json = j;
}

void rf::from_json(const nlohmann::ordered_json& json, MixGroupState& object)
{
    object.m_volumeDb = json["volumeDb"];
    object.m_isMaster = json["isMaster"];
}