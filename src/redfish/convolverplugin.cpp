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

#include "convolverplugin.h"

#include "assetsystem.h"
#include "context.h"
#include "convolverdsp.h"
#include "functions.h"
#include "plugincommands.h"
#include "pluginutils.h"

rf::ConvolverPlugin::ConvolverPlugin(Context* context, CommandProcessor* commands, MixGroupHandle mixGroupHandle, int mixGroupSlot, int pluginIndex)
    : PluginBase(context, commands, mixGroupHandle, mixGroupSlot, pluginIndex)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreateConvolverDSPCommand);

    m_amplitudes = Allocator::AllocateArray<float>("ConvolverPluginAmplitudes", PluginUtils::k_maxConvolverIRs);
    for (int i = 0; i < PluginUtils::k_maxConvolverIRs; ++i)
    {
        m_amplitudes[i] = 1.0f;
    }
}

rf::ConvolverPlugin::~ConvolverPlugin()
{
    Allocator::DeallocateArray<float>(&m_amplitudes, PluginUtils::k_maxConvolverIRs);
    RF_SEND_PLUGIN_DESTROY_COMMAND(DestroyConvolverDSPCommand);
}

void rf::ConvolverPlugin::LoadIR(const AudioHandle audioHandle, int index)
{
    if (!ConvolverDSP::IndexCheck(index))
    {
        return;
    }

    AudioCommand cmd;
    LoadConvolverDSPIRCommand& data = EncodeAudioCommand<LoadConvolverDSPIRCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_index = index;
    data.m_amplitude = m_amplitudes[index];
    data.m_audioDataIndex = m_context->GetAssetSystem()->GetAudioDataIndex(audioHandle);
    m_commands->Add(cmd);
}

void rf::ConvolverPlugin::UnloadIR(int index)
{
    if (!ConvolverDSP::IndexCheck(index))
    {
        return;
    }

    AudioCommand cmd;
    UnloadConvolverDSPIRCommand& data = EncodeAudioCommand<UnloadConvolverDSPIRCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_index = index;
    m_commands->Add(cmd);
}

void rf::ConvolverPlugin::SetWetPercentage(float percentage)
{
    if (Functions::FloatEquality(m_wetPercentage, percentage))
    {
        return;
    }

    m_wetPercentage = Functions::Clamp(percentage, 0.0f, 1.0f);

    AudioCommand cmd;
    SetConvolverDSPWetPercentageCommand& data = EncodeAudioCommand<SetConvolverDSPWetPercentageCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_percentage = m_wetPercentage;
    m_commands->Add(cmd);
}

float rf::ConvolverPlugin::GetWetPercentage() const
{
    return m_wetPercentage;
}

void rf::ConvolverPlugin::SetIRVolumeDb(float volumeDb, int index)
{
    if (!ConvolverDSP::IndexCheck(index))
    {
        return;
    }

    const float amplitude = Functions::DecibelToAmplitude(volumeDb);
    if (Functions::FloatEquality(m_amplitudes[index], amplitude))
    {
        return;
    }

    m_amplitudes[index] = amplitude;

    AudioCommand cmd;
    SetConvolverDSPIRAmplitudeCommand& data = EncodeAudioCommand<SetConvolverDSPIRAmplitudeCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_index = index;
    data.m_amplitude = m_amplitudes[index];
    m_commands->Add(cmd);
}

float rf::ConvolverPlugin::GetIRVolumeDb(int index) const
{
    if (!ConvolverDSP::IndexCheck(index))
    {
        return 0.0f;
    }

    const float dB = Functions::AmplitudeToDecibel(m_amplitudes[index]);
    return dB;
}