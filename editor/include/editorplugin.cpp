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

#include "editorplugin.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>
#include <stdio.h>

rf::EditorPlugin::EditorPlugin(EditorPlugin::Type type)
    : m_type(type)
{
}

const char* rf::EditorPlugin::GetPluginName(Type type)
{
    static_assert(static_cast<int>(EditorPlugin::Type::Version) == 1, "Update this");

    switch (type)
    {
        case Type::ButterworthHighpassFilter: return "Butterworth HP Filter";
        case Type::ButterworthLowpassFilter: return "Butterworth LP Filter";
        case Type::Compressor: return "Compressor";
        case Type::Convolver: return "Convolver";
        case Type::Delay: return "Delay";
        case Type::Gain: return "Gain";
        case Type::IIR2HighpassFilter: return "IIR2 HP Filter";
        case Type::IIR2LowpassFilter: return "IIR2 LP Filter";
        case Type::Limiter: return "Limiter";
        case Type::Pan: return "Pan";
        case Type::Positioning: return "Positioning";
        default: RF_FAIL("Unhandled case"); return "";
    }
}

const char* rf::EditorPlugin::GetName() const
{
    return GetPluginName(m_type);
}

void rf::EditorPlugin::BuildName(char* buffer, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot) const
{
    sprintf(buffer, "%s:%i:%s", mixerSystem->GetMixGroup(mixGroupHandle)->GetName(), slot, GetName());
}