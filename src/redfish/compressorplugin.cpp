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

#include "compressorplugin.h"

#include "commandprocessor.h"
#include "defines.h"
#include "functions.h"
#include "plugincommands.h"
#include "pluginutils.h"

rf::CompressorPlugin::CompressorPlugin(Context* context, CommandProcessor* commands, MixGroupHandle mixGroupHandle, int mixGroupSlot, int pluginIndex)
    : PluginBase(context, commands, mixGroupHandle, mixGroupSlot, pluginIndex)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreateCompressorDSPCommand);
}

rf::CompressorPlugin::~CompressorPlugin()
{
    RF_SEND_PLUGIN_DESTROY_COMMAND(DestroyCompressorDSPCommand);
}

void rf::CompressorPlugin::SetThreshold(float threshold)
{
    if (Functions::FloatEquality(m_threshold, threshold))
    {
        return;
    }

    m_threshold = Functions::Clamp(threshold, RF_MIN_DECIBELS, RF_MAX_DECIBELS);

    AudioCommand cmd;
    SetCompressorDSPThresholdCommand& data = EncodeAudioCommand<SetCompressorDSPThresholdCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_threshold = m_threshold;
    m_commands->Add(cmd);
}

float rf::CompressorPlugin::GetThreshold() const
{
    return m_threshold;
}

void rf::CompressorPlugin::SetRatio(float ratio)
{
    if (Functions::FloatEquality(m_ratio, ratio))
    {
        return;
    }

    m_ratio = Functions::Clamp(ratio, 1.0f, 100.0f);

    AudioCommand cmd;
    SetCompressorDSPRatioCommand& data = EncodeAudioCommand<SetCompressorDSPRatioCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_ratio = m_ratio;
    m_commands->Add(cmd);
}

float rf::CompressorPlugin::GetRatio() const
{
    return m_ratio;
}

void rf::CompressorPlugin::SetMakeUpGainDb(float makeUpGainDb)
{
    if (Functions::FloatEquality(m_makeUpGainDb, makeUpGainDb))
    {
        return;
    }

    m_makeUpGainDb = Functions::Clamp(makeUpGainDb, RF_MIN_DECIBELS, RF_MAX_DECIBELS);

    AudioCommand cmd;
    SetCompressorDSPMakeUpGainAmplitudeCommand& data = EncodeAudioCommand<SetCompressorDSPMakeUpGainAmplitudeCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_amplitude = Functions::DecibelToAmplitude(m_makeUpGainDb);
    m_commands->Add(cmd);
}

float rf::CompressorPlugin::GetMakeUpGainDb() const
{
    return m_makeUpGainDb;
}

void rf::CompressorPlugin::SetAttack(float attack)
{
    if (Functions::FloatEquality(m_attack, attack))
    {
        return;
    }

    m_attack = Functions::Clamp(attack, 0.0f, 500.0f);

    AudioCommand cmd;
    SetCompressorDSPAttackCommand& data = EncodeAudioCommand<SetCompressorDSPAttackCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_attack = m_attack;
    m_commands->Add(cmd);
}

float rf::CompressorPlugin::GetAttack() const
{
    return m_attack;
}

void rf::CompressorPlugin::SetRelease(float release)
{
    if (Functions::FloatEquality(m_release, release))
    {
        return;
    }

    m_release = Functions::Clamp(release, 0.0f, 5000.0f);

    AudioCommand cmd;
    SetCompressorDSPReleaseCommand& data = EncodeAudioCommand<SetCompressorDSPReleaseCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_release = m_release;
    m_commands->Add(cmd);
}

float rf::CompressorPlugin::GetRelease() const
{
    return m_release;
}