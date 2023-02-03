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

#include "plugincommands.h"

#include "audiotimeline.h"
#include "butterworthhighpassfilterdsp.h"
#include "butterworthlowpassfilterdsp.h"
#include "compressordsp.h"
#include "convolverdsp.h"
#include "delaydsp.h"
#include "gaindsp.h"
#include "iir2highpassfilterdsp.h"
#include "iir2lowpassfilterdsp.h"
#include "limiterdsp.h"
#include "pandsp.h"
#include "positioningdsp.h"

#define RF_CREATE_DSP(dspName)                                                                                       \
    rf::AudioCommandCallback rf::Create##dspName##Command::s_callback = [](AudioTimeline* timeline, void* command) { \
        const Create##dspName##Command& cmd = *static_cast<Create##dspName##Command*>(command);                      \
        SummingMixer* mixer = &timeline->m_summingMixer;                                                             \
        RF_ASSERT(!mixer->m_dsp[cmd.m_dspIndex], "Expected nullptr");                                                \
        mixer->m_dsp[cmd.m_dspIndex] = Allocator::Allocate<dspName>(#dspName, timeline->GetAudioSpec());             \
        SummingMixer::MixGroupInternal* mixGroup = mixer->MixGroupLookUp(cmd.m_mixGroupHandle);                      \
        mixGroup->m_state.m_pluginSlots[cmd.m_mixGroupSlot] = cmd.m_dspIndex;                                        \
    };

#define RF_DESTROY_DSP(dspName)                                                                                       \
    rf::AudioCommandCallback rf::Destroy##dspName##Command::s_callback = [](AudioTimeline* timeline, void* command) { \
        const Destroy##dspName##Command& cmd = *static_cast<Destroy##dspName##Command*>(command);                     \
        SummingMixer* mixer = &timeline->m_summingMixer;                                                              \
        RF_ASSERT(mixer->m_dsp[cmd.m_dspIndex], "Expected a pointer");                                                \
        Allocator::Deallocate<DSPBase>(&mixer->m_dsp[cmd.m_dspIndex]);                                                \
        SummingMixer::MixGroupInternal* mixGroup = mixer->MixGroupLookUp(cmd.m_mixGroupHandle);                       \
        mixGroup->m_state.m_pluginSlots[cmd.m_mixGroupSlot] = -1;                                                     \
    };

#define RF_SET_DSP_PARAMETER(dspName, function, parameter)                                                                  \
    rf::AudioCommandCallback rf::Set##dspName##function##Command::s_callback = [](AudioTimeline* timeline, void* command) { \
        const Set##dspName##function##Command& cmd = *static_cast<Set##dspName##function##Command*>(command);               \
        SummingMixer* mixer = &timeline->m_summingMixer;                                                                    \
        dspName* dsp = static_cast<dspName*>(mixer->m_dsp[cmd.m_dspIndex]);                                                 \
        dsp->Set##function(cmd.parameter);                                                                                  \
    };

rf::AudioCommandCallback rf::SetDSPBypassCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SetDSPBypassCommand& cmd = *static_cast<SetDSPBypassCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    mixer->m_dsp[cmd.m_dspIndex]->SetBypass(cmd.m_bypass);
};

RF_CREATE_DSP(GainDSP);
RF_DESTROY_DSP(GainDSP);
RF_SET_DSP_PARAMETER(GainDSP, Amplitude, m_amplitude);

RF_CREATE_DSP(PanDSP);
RF_DESTROY_DSP(PanDSP);
RF_SET_DSP_PARAMETER(PanDSP, Angle, m_angle);

RF_CREATE_DSP(ButterworthHighpassFilterDSP);
RF_DESTROY_DSP(ButterworthHighpassFilterDSP);
RF_SET_DSP_PARAMETER(ButterworthHighpassFilterDSP, Order, m_order);
RF_SET_DSP_PARAMETER(ButterworthHighpassFilterDSP, Cutoff, m_cutoff);

RF_CREATE_DSP(ButterworthLowpassFilterDSP);
RF_DESTROY_DSP(ButterworthLowpassFilterDSP);
RF_SET_DSP_PARAMETER(ButterworthLowpassFilterDSP, Order, m_order);
RF_SET_DSP_PARAMETER(ButterworthLowpassFilterDSP, Cutoff, m_cutoff);

RF_CREATE_DSP(IIR2LowpassFilterDSP);
RF_DESTROY_DSP(IIR2LowpassFilterDSP);
RF_SET_DSP_PARAMETER(IIR2LowpassFilterDSP, Q, m_q);
RF_SET_DSP_PARAMETER(IIR2LowpassFilterDSP, Cutoff, m_cutoff);

RF_CREATE_DSP(IIR2HighpassFilterDSP);
RF_DESTROY_DSP(IIR2HighpassFilterDSP);
RF_SET_DSP_PARAMETER(IIR2HighpassFilterDSP, Q, m_q);
RF_SET_DSP_PARAMETER(IIR2HighpassFilterDSP, Cutoff, m_cutoff);

RF_CREATE_DSP(DelayDSP);
RF_DESTROY_DSP(DelayDSP);
RF_SET_DSP_PARAMETER(DelayDSP, Delay, m_delay);
RF_SET_DSP_PARAMETER(DelayDSP, Feedback, m_feedback);

RF_CREATE_DSP(LimiterDSP);
RF_DESTROY_DSP(LimiterDSP);
RF_SET_DSP_PARAMETER(LimiterDSP, Threshold, m_threshold);

RF_CREATE_DSP(CompressorDSP);
RF_DESTROY_DSP(CompressorDSP);
RF_SET_DSP_PARAMETER(CompressorDSP, Threshold, m_threshold);
RF_SET_DSP_PARAMETER(CompressorDSP, Ratio, m_ratio);
RF_SET_DSP_PARAMETER(CompressorDSP, MakeUpGainAmplitude, m_amplitude);
RF_SET_DSP_PARAMETER(CompressorDSP, Attack, m_attack);
RF_SET_DSP_PARAMETER(CompressorDSP, Release, m_release);

RF_CREATE_DSP(ConvolverDSP);
RF_DESTROY_DSP(ConvolverDSP);
RF_SET_DSP_PARAMETER(ConvolverDSP, WetPercentage, m_percentage);

rf::AudioCommandCallback rf::LoadConvolverDSPIRCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const LoadConvolverDSPIRCommand& cmd = *static_cast<LoadConvolverDSPIRCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    ConvolverDSP* dsp = static_cast<ConvolverDSP*>(mixer->m_dsp[cmd.m_dspIndex]);
    dsp->LoadIR(timeline->GetAudioData(cmd.m_audioDataIndex), cmd.m_amplitude, cmd.m_index);
};

rf::AudioCommandCallback rf::UnloadConvolverDSPIRCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const UnloadConvolverDSPIRCommand& cmd = *static_cast<UnloadConvolverDSPIRCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    ConvolverDSP* dsp = static_cast<ConvolverDSP*>(mixer->m_dsp[cmd.m_dspIndex]);
    dsp->UnloadIR(cmd.m_index);
};

rf::AudioCommandCallback rf::SetConvolverDSPIRAmplitudeCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SetConvolverDSPIRAmplitudeCommand& cmd = *static_cast<SetConvolverDSPIRAmplitudeCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    ConvolverDSP* dsp = static_cast<ConvolverDSP*>(mixer->m_dsp[cmd.m_dspIndex]);
    dsp->SetIRAmplitude(cmd.m_amplitude, cmd.m_index);
};

RF_CREATE_DSP(PositioningDSP);
RF_DESTROY_DSP(PositioningDSP);
RF_SET_DSP_PARAMETER(PositioningDSP, PositioningParameters, m_parameters);

#undef RF_CREATE_DSP
#undef RF_DESTROY_DSP
#undef RF_SET_DSP_PARAMETER
