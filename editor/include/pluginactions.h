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
#include <redfish/positioningparameters.h>

#include "action.h"

#define RF_PLUGIN_GUI_BOOL(label, currentValueGetter, actionName)                                                           \
    {                                                                                                                       \
        const bool currentValue = currentValueGetter();                                                                     \
        bool value = currentValue;                                                                                          \
        if (ImGui::Checkbox(label, &value))                                                                                 \
        {                                                                                                                   \
            actions->DoAction(new actionName(this, mixerSystem, mixGroup->GetMixGroupHandle(), slot, currentValue, value)); \
        }                                                                                                                   \
    }

#define RF_PLUGIN_GUI_FLOAT_SLIDER(label, speed, min, max, currentValueGetter, setter, actionName)                          \
    {                                                                                                                       \
        const float currentValue = currentValueGetter();                                                                    \
        static float s_firstValue;                                                                                          \
        static bool s_first = true;                                                                                         \
        float value = currentValue;                                                                                         \
        const bool active = ImGui::DragFloat(label, &value, speed, min, max);                                               \
        const bool end = ImGui::IsItemDeactivatedAfterEdit();                                                               \
        setter(value);                                                                                                      \
        if (active && s_first)                                                                                              \
        {                                                                                                                   \
            s_firstValue = currentValue;                                                                                    \
            s_first = false;                                                                                                \
        }                                                                                                                   \
        if (end)                                                                                                            \
        {                                                                                                                   \
            actions->DoAction(new actionName(this, mixerSystem, mixGroup->GetMixGroupHandle(), slot, s_firstValue, value)); \
            s_first = true;                                                                                                 \
        }                                                                                                                   \
    }

#define RF_PLUGIN_GUI_INT_SLIDER(label, speed, min, max, currentValueGetter, setter, actionName)                            \
    {                                                                                                                       \
        const int currentValue = currentValueGetter();                                                                      \
        static int s_firstValue;                                                                                            \
        static bool s_first = true;                                                                                         \
        int value = currentValue;                                                                                           \
        const bool active = ImGui::DragInt(label, &value, speed, min, max);                                                 \
        const bool end = ImGui::IsItemDeactivatedAfterEdit();                                                               \
        setter(value);                                                                                                      \
        if (active && s_first)                                                                                              \
        {                                                                                                                   \
            s_firstValue = currentValue;                                                                                    \
            s_first = false;                                                                                                \
        }                                                                                                                   \
        if (end)                                                                                                            \
        {                                                                                                                   \
            actions->DoAction(new actionName(this, mixerSystem, mixGroup->GetMixGroupHandle(), slot, s_firstValue, value)); \
            s_first = true;                                                                                                 \
        }                                                                                                                   \
    }

#define RF_DEFINE_FLOAT_SETTER(name)                                                                                                    \
    class name final : public SetFloatParameters                                                                                        \
    {                                                                                                                                   \
    public:                                                                                                                             \
        name(EditorPlugin* editorPlugin, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, float before, float after); \
        void Do() override;                                                                                                             \
        void Undo() override;                                                                                                           \
    };

#define RF_DEFINE_INT_SETTER(name)                                                                                                  \
    class name final : public SetIntParameter                                                                                       \
    {                                                                                                                               \
    public:                                                                                                                         \
        name(EditorPlugin* editorPlugin, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, int before, int after); \
        void Do() override;                                                                                                         \
        void Undo() override;                                                                                                       \
    };

namespace rf
{
class EditorPlugin;
class MixerSystem;

class SetBypassAction final : public Action
{
public:
    SetBypassAction(EditorPlugin* editorPlugin, MixerSystem* mixerSystem, MixGroupHandle mixGroupHandle, int slot, bool before, bool after);

    void Do() override;
    void Undo() override;

protected:
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    int m_slot = -1;
    bool m_before = false;
    bool m_after = false;
};

class SetPositioningParamsAction final : public Action
{
public:
    SetPositioningParamsAction(EditorPlugin* editorPlugin,
                               MixerSystem* mixerSystem,
                               MixGroupHandle mixGroupHandle,
                               int slot,
                               PositioningParameters before,
                               PositioningParameters after);

    void Do() override;
    void Undo() override;

protected:
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    int m_slot = -1;
    PositioningParameters m_before;
    PositioningParameters m_after;
};

class SetFloatParameters : public Action
{
public:
    SetFloatParameters(EditorPlugin* editorPlugin,
                       const char* paramName,
                       MixerSystem* mixerSystem,
                       MixGroupHandle mixGroupHandle,
                       int slot,
                       float before,
                       float after);

    void Do() override = 0;
    void Undo() override = 0;

protected:
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    int m_slot = -1;
    float m_before = 0.0f;
    float m_after = 0.0f;
};

class SetIntParameter : public Action
{
public:
    SetIntParameter(EditorPlugin* editorPlugin,
                    const char* paramName,
                    MixerSystem* mixerSystem,
                    MixGroupHandle mixGroupHandle,
                    int slot,
                    int before,
                    int after);

    void Do() override = 0;
    void Undo() override = 0;

protected:
    MixerSystem* m_mixerSystem = nullptr;
    MixGroupHandle m_mixGroupHandle;
    int m_slot = -1;
    int m_before = 0;
    int m_after = 0;
};

RF_DEFINE_FLOAT_SETTER(Gain_SetGain);
RF_DEFINE_INT_SETTER(BWHP_SetOrder);
RF_DEFINE_FLOAT_SETTER(BWHP_SetCutoff);
RF_DEFINE_INT_SETTER(BWLP_SetOrder);
RF_DEFINE_FLOAT_SETTER(BWLP_SetCutoff);
RF_DEFINE_FLOAT_SETTER(Compressor_SetThreshold);
RF_DEFINE_FLOAT_SETTER(Compressor_SetRatio);
RF_DEFINE_FLOAT_SETTER(Compressor_SetMakeUpGain);
RF_DEFINE_FLOAT_SETTER(Compressor_SetAttack);
RF_DEFINE_FLOAT_SETTER(Compressor_SetRelease);
RF_DEFINE_FLOAT_SETTER(Delay_SetDelay);
RF_DEFINE_FLOAT_SETTER(Delay_SetFeedback);
RF_DEFINE_FLOAT_SETTER(IIR2HP_SetQ);
RF_DEFINE_FLOAT_SETTER(IIR2HP_SetCutoff);
RF_DEFINE_FLOAT_SETTER(IIR2LP_SetQ);
RF_DEFINE_FLOAT_SETTER(IIR2LP_SetCutoff);
RF_DEFINE_FLOAT_SETTER(Limiter_SetThreshold);
RF_DEFINE_FLOAT_SETTER(Pan_SetAngle);
RF_DEFINE_FLOAT_SETTER(Position_SetAngle);
RF_DEFINE_FLOAT_SETTER(Position_SetCurrentDistance);
RF_DEFINE_FLOAT_SETTER(Position_MinCurrentDistance);
RF_DEFINE_FLOAT_SETTER(Position_MaxCurrentDistance);
RF_DEFINE_FLOAT_SETTER(Position_MaxAttenuationDb);
RF_DEFINE_FLOAT_SETTER(Position_MaxHPFCutoff);
RF_DEFINE_FLOAT_SETTER(Position_MaxLPFCutoff);
}  // namespace rf

#undef RF_DEFINE_FLOAT_SETTER
#undef RF_DEFINE_INT_SETTER