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

#include "pluginactions.h"

#include <redfish/butterworthhighpassfilterplugin.h>
#include <redfish/butterworthlowpassfilterplugin.h>
#include <redfish/compressorplugin.h>
#include <redfish/delayplugin.h>
#include <redfish/gainplugin.h>
#include <redfish/iir2highpassfilterplugin.h>
#include <redfish/iir2lowpassfilterplugin.h>
#include <redfish/limiterplugin.h>
#include <redfish/mixersystem.h>
#include <redfish/mixgroup.h>
#include <redfish/panplugin.h>
#include <redfish/positioningplugin.h>

#include "editorplugin.h"

#define RF_GET_PLUGIN(type) m_mixerSystem->GetMixGroup(m_mixGroupHandle)->GetPlugin<type>(m_slot)

#define RF_IMPLEMENT_FLOAT_SETTER(name, paramName, pluginType, setter)                                                                       \
    rf::name::name(EditorPlugin* editorPlugin, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, float before, float after) \
        : SetFloatParameters(editorPlugin, paramName, mixerSystem, mixGroupHandle, slot, before, after)                                      \
    {                                                                                                                                        \
    }                                                                                                                                        \
    void rf::name::Do()                                                                                                                      \
    {                                                                                                                                        \
        RF_GET_PLUGIN(pluginType)->setter(m_after);                                                                                          \
    }                                                                                                                                        \
    void rf::name::Undo()                                                                                                                    \
    {                                                                                                                                        \
        RF_GET_PLUGIN(pluginType)->setter(m_before);                                                                                         \
    }

#define RF_IMPLEMENT_INT_SETTER(name, paramName, pluginType, setter)                                                                     \
    rf::name::name(EditorPlugin* editorPlugin, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, int before, int after) \
        : SetIntParameter(editorPlugin, paramName, mixerSystem, mixGroupHandle, slot, before, after)                                     \
    {                                                                                                                                    \
    }                                                                                                                                    \
    void rf::name::Do()                                                                                                                  \
    {                                                                                                                                    \
        RF_GET_PLUGIN(pluginType)->setter(m_after);                                                                                      \
    }                                                                                                                                    \
    void rf::name::Undo()                                                                                                                \
    {                                                                                                                                    \
        RF_GET_PLUGIN(pluginType)->setter(m_before);                                                                                     \
    }

rf::SetBypassAction::SetBypassAction(EditorPlugin* editorPlugin,
                                     MixerSystem* mixerSystem,
                                     MixGroupHandle mixGroupHandle,
                                     int slot,
                                     bool before,
                                     bool after)
    : m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_slot(slot)
    , m_before(before)
    , m_after(after)
{
    char bufferName[128] = {};
    editorPlugin->BuildName(bufferName, mixerSystem, mixGroupHandle, slot);
    sprintf_s(m_name, "Set '%s' Bypass from %d to %d", bufferName, m_before, m_after);
}

void rf::SetBypassAction::Do()
{
    m_mixerSystem->GetMixGroup(m_mixGroupHandle)->GetPlugin(m_slot)->SetBypass(m_after);
}

void rf::SetBypassAction::Undo()
{
    m_mixerSystem->GetMixGroup(m_mixGroupHandle)->GetPlugin(m_slot)->SetBypass(m_before);
}

rf::SetPositioningParamsAction::SetPositioningParamsAction(EditorPlugin* editorPlugin,
                                                           MixerSystem* mixerSystem,
                                                           MixGroupHandle mixGroupHandle,
                                                           int slot,
                                                           PositioningParameters before,
                                                           PositioningParameters after)
    : m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_slot(slot)
    , m_before(before)
    , m_after(after)
{
    char bufferName[128] = {};
    editorPlugin->BuildName(bufferName, mixerSystem, mixGroupHandle, slot);
    sprintf_s(m_name, "Set '%s' Positioning Params", bufferName);
}

void rf::SetPositioningParamsAction::Do()
{
    RF_GET_PLUGIN(PositioningPlugin)->SetPositioningParameters(m_after);
}

void rf::SetPositioningParamsAction::Undo()
{
    RF_GET_PLUGIN(PositioningPlugin)->SetPositioningParameters(m_before);
}

rf::SetFloatParameters::SetFloatParameters(EditorPlugin* editorPlugin,
                                           const char* paramName,
                                           MixerSystem* mixerSystem,
                                           MixGroupHandle mixGroupHandle,
                                           int slot,
                                           float before,
                                           float after)
    : m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_slot(slot)
    , m_before(before)
    , m_after(after)
{
    char bufferName[128] = {};
    editorPlugin->BuildName(bufferName, mixerSystem, mixGroupHandle, slot);
    sprintf_s(m_name, "Set '%s' %s from %.2f to %.2f", bufferName, paramName, m_before, m_after);
}

rf::SetIntParameter::SetIntParameter(EditorPlugin* editorPlugin,
                                     const char* paramName,
                                     MixerSystem* mixerSystem,
                                     MixGroupHandle mixGroupHandle,
                                     int slot,
                                     int before,
                                     int after)
    : m_mixerSystem(mixerSystem)
    , m_mixGroupHandle(mixGroupHandle)
    , m_slot(slot)
    , m_before(before)
    , m_after(after)
{
    char bufferName[128] = {};
    editorPlugin->BuildName(bufferName, mixerSystem, mixGroupHandle, slot);
    sprintf_s(m_name, "Set '%s' %s from %i to %i", bufferName, paramName, m_before, m_after);
}

RF_IMPLEMENT_FLOAT_SETTER(Gain_SetGain, "Gain dB", GainPlugin, SetGainDb);
RF_IMPLEMENT_INT_SETTER(BWHP_SetOrder, "Order", ButterworthHighpassFilterPlugin, SetOrder);
RF_IMPLEMENT_FLOAT_SETTER(BWHP_SetCutoff, "Cutoff", ButterworthHighpassFilterPlugin, SetCutoff);
RF_IMPLEMENT_INT_SETTER(BWLP_SetOrder, "Order", ButterworthLowpassFilterPlugin, SetOrder);
RF_IMPLEMENT_FLOAT_SETTER(BWLP_SetCutoff, "Cutoff", ButterworthLowpassFilterPlugin, SetCutoff);
RF_IMPLEMENT_FLOAT_SETTER(Compressor_SetThreshold, "Threshold", CompressorPlugin, SetThreshold);
RF_IMPLEMENT_FLOAT_SETTER(Compressor_SetRatio, "Ratio", CompressorPlugin, SetRatio);
RF_IMPLEMENT_FLOAT_SETTER(Compressor_SetMakeUpGain, "Make Up Gain dB", CompressorPlugin, SetMakeUpGainDb);
RF_IMPLEMENT_FLOAT_SETTER(Compressor_SetAttack, "Attack ms", CompressorPlugin, SetAttack);
RF_IMPLEMENT_FLOAT_SETTER(Compressor_SetRelease, "Release ms", CompressorPlugin, SetRelease);
RF_IMPLEMENT_FLOAT_SETTER(Delay_SetDelay, "Delay ms", DelayPlugin, SetDelay);
RF_IMPLEMENT_FLOAT_SETTER(Delay_SetFeedback, "Feedback", DelayPlugin, SetFeedback);
RF_IMPLEMENT_FLOAT_SETTER(IIR2HP_SetQ, "Q", IIR2HighpassFilterPlugin, SetQ);
RF_IMPLEMENT_FLOAT_SETTER(IIR2HP_SetCutoff, "Cutoff", IIR2HighpassFilterPlugin, SetCutoff);
RF_IMPLEMENT_FLOAT_SETTER(IIR2LP_SetQ, "Q", IIR2LowpassFilterPlugin, SetQ);
RF_IMPLEMENT_FLOAT_SETTER(IIR2LP_SetCutoff, "Cutoff", IIR2LowpassFilterPlugin, SetCutoff);
RF_IMPLEMENT_FLOAT_SETTER(Limiter_SetThreshold, "Threshold", LimiterPlugin, SetThreshold);
RF_IMPLEMENT_FLOAT_SETTER(Pan_SetAngle, "Pan", PanPlugin, SetAngle);

#undef RF_GET_PLUGIN
#undef RF_IMPLEMENT_FLOAT_SETTER
#undef RF_IMPLEMENT_INT_SETTER