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

#include "pluginbase.h"

#include "commandprocessor.h"
#include "plugincommands.h"

rf::PluginBase::PluginBase(Context* context, CommandProcessor* commands, int pluginIndex)
    : m_context(context)
    , m_commands(commands)
    , m_pluginIndex(pluginIndex)
{
}

void rf::PluginBase::SetBypass(bool bypass)
{
    if (m_bypass == bypass)
    {
        return;
    }

    m_bypass = bypass;

    AudioCommand cmd;
    SetDSPBypassCommand& data = EncodeAudioCommand<SetDSPBypassCommand>(&cmd);
    data.m_dspIndex = m_pluginIndex;
    data.m_bypass = m_bypass;
    m_commands->Add(cmd);
}

bool rf::PluginBase::GetBypass() const
{
    return m_bypass;
}
