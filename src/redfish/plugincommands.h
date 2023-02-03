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
#include "audiocommand.h"
#include "identifiers.h"
#include "positioningparameters.h"

namespace rf
{
struct CreateCommand
{
    int m_dspIndex = -1;
    int m_mixGroupSlot = -1;
    MixGroupHandle m_mixGroupHandle;
};

struct SetDSPBypassCommand
{
    int m_dspIndex = -1;
    bool m_bypass = false;
    static AudioCommandCallback s_callback;
};

struct CreateGainDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyGainDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetGainDSPAmplitudeCommand
{
    int m_dspIndex = -1;
    float m_amplitude = 1.0f;
    static AudioCommandCallback s_callback;
};

struct CreatePanDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyPanDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetPanDSPAngleCommand
{
    int m_dspIndex = -1;
    float m_angle = 1.0f;
    static AudioCommandCallback s_callback;
};

struct CreateButterworthHighpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyButterworthHighpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetButterworthHighpassFilterDSPOrderCommand
{
    int m_dspIndex = -1;
    int m_order = -1;
    static AudioCommandCallback s_callback;
};

struct SetButterworthHighpassFilterDSPCutoffCommand
{
    int m_dspIndex = -1;
    float m_cutoff = -1.0f;
    static AudioCommandCallback s_callback;
};

struct CreateButterworthLowpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyButterworthLowpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetButterworthLowpassFilterDSPOrderCommand
{
    int m_dspIndex = -1;
    int m_order = -1;
    static AudioCommandCallback s_callback;
};

struct SetButterworthLowpassFilterDSPCutoffCommand
{
    int m_dspIndex = -1;
    float m_cutoff = -1.0f;
    static AudioCommandCallback s_callback;
};

struct CreateIIR2HighpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyIIR2HighpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetIIR2HighpassFilterDSPQCommand
{
    int m_dspIndex = -1;
    float m_q = -1;
    static AudioCommandCallback s_callback;
};

struct SetIIR2HighpassFilterDSPCutoffCommand
{
    int m_dspIndex = -1;
    float m_cutoff = -1.0f;
    static AudioCommandCallback s_callback;
};

struct CreateIIR2LowpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyIIR2LowpassFilterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetIIR2LowpassFilterDSPQCommand
{
    int m_dspIndex = -1;
    float m_q = -1;
    static AudioCommandCallback s_callback;
};

struct SetIIR2LowpassFilterDSPCutoffCommand
{
    int m_dspIndex = -1;
    float m_cutoff = -1.0f;
    static AudioCommandCallback s_callback;
};

struct CreateDelayDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyDelayDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetDelayDSPDelayCommand
{
    int m_dspIndex = -1;
    int m_delay = -1;
    static AudioCommandCallback s_callback;
};

struct SetDelayDSPFeedbackCommand
{
    int m_dspIndex = -1;
    float m_feedback = -1.0f;
    static AudioCommandCallback s_callback;
};

struct CreateLimiterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyLimiterDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetLimiterDSPThresholdCommand
{
    int m_dspIndex = -1;
    float m_threshold = -1;
    static AudioCommandCallback s_callback;
};

struct CreateCompressorDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyCompressorDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetCompressorDSPThresholdCommand
{
    int m_dspIndex = -1;
    float m_threshold = -1;
    static AudioCommandCallback s_callback;
};

struct SetCompressorDSPRatioCommand
{
    int m_dspIndex = -1;
    float m_ratio = -1;
    static AudioCommandCallback s_callback;
};

struct SetCompressorDSPMakeUpGainAmplitudeCommand
{
    int m_dspIndex = -1;
    float m_amplitude = -1;
    static AudioCommandCallback s_callback;
};

struct SetCompressorDSPAttackCommand
{
    int m_dspIndex = -1;
    float m_attack = -1;
    static AudioCommandCallback s_callback;
};

struct SetCompressorDSPReleaseCommand
{
    int m_dspIndex = -1;
    float m_release = -1;
    static AudioCommandCallback s_callback;
};

struct CreateConvolverDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyConvolverDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetConvolverDSPWetPercentageCommand
{
    int m_dspIndex = -1;
    float m_percentage = 1.0f;
    static AudioCommandCallback s_callback;
};

struct LoadConvolverDSPIRCommand
{
    int m_dspIndex = -1;
    int m_index = -1;
    float m_amplitude = 1.0f;
    int m_audioDataIndex = -1;
    static AudioCommandCallback s_callback;
};

struct UnloadConvolverDSPIRCommand
{
    int m_dspIndex = -1;
    int m_index = -1;
    static AudioCommandCallback s_callback;
};

struct SetConvolverDSPIRAmplitudeCommand
{
    int m_dspIndex = -1;
    int m_index = -1;
    float m_amplitude = 1.0f;
    static AudioCommandCallback s_callback;
};

struct CreatePositioningDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct DestroyPositioningDSPCommand : public CreateCommand
{
    static AudioCommandCallback s_callback;
};

struct SetPositioningDSPPositioningParametersCommand
{
    int m_dspIndex = -1;
    PositioningParameters m_parameters;
    static AudioCommandCallback s_callback;
};
}  // namespace rf