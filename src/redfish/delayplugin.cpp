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

#include "delayplugin.h"

#include "context.h"
#include "functions.h"
#include "plugincommands.h"
#include "pluginutils.h"

rf::DelayPlugin::DelayPlugin(Context* context, CommandProcessor* commands, int pluginIndex, int mixGroupSlot, MixGroupHandle mixGroupHandle)
    : PluginBase(context, commands, pluginIndex)
{
    RF_SEND_PLUGIN_CREATE_COMMAND(CreateDelayDSPCommand);
}

void rf::DelayPlugin::SetDelay(float delay)
{
    if (Functions::FloatEquality(m_delay, delay))
    {
        return;
    }

    m_delay = Functions::Clamp(delay, 0.0f, PluginUtils::k_maxDelayTime);

    AudioCommand cmd;
    SetDelayDSPDelayCommand& data = EncodeAudioCommand<SetDelayDSPDelayCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_delay = Functions::MsToSamples(m_delay, m_context->GetAudioSpec().m_sampleRate);
    m_commands->Add(cmd);
}

float rf::DelayPlugin::GetDelay() const
{
    return m_delay;
}

void rf::DelayPlugin::SetFeedback(float feedback)
{
    if (Functions::FloatEquality(m_feedback, feedback))
    {
        return;
    }

    m_feedback = Functions::Clamp(feedback, PluginUtils::k_minDelayFeedback, PluginUtils::k_maxDelayFeedback);

    AudioCommand cmd;
    SetDelayDSPFeedbackCommand& data = EncodeAudioCommand<SetDelayDSPFeedbackCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_feedback = m_feedback;
    m_commands->Add(cmd);
}

float rf::DelayPlugin::GetFeedback() const
{
    return m_feedback;
}
