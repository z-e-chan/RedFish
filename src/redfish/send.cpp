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

#include "send.h"

#include "commandprocessor.h"
#include "functions.h"
#include "mixercommands.h"

rf::Send::Send(CommandProcessor* commands, int sendIndex, MixGroupHandle sendToMixGroupHandle)
    : m_commands(commands)
    , m_sendIndex(sendIndex)
    , m_sendToMixGroupHandle(sendToMixGroupHandle)
{
}

rf::Send::operator bool() const
{
    return static_cast<bool>(m_sendToMixGroupHandle);
}

void rf::Send::SetVolumeDb(float volumeDb)
{
    const float amplitude = Functions::DecibelToAmplitude(volumeDb);
    if (Functions::FloatEquality(m_amplitude, amplitude))
    {
        return;
    }

    m_amplitude = amplitude;

    AudioCommand cmd;
    SetSendAmplitudeCommand& data = EncodeAudioCommand<SetSendAmplitudeCommand>(&cmd);
    data.m_sendIndex = m_sendIndex;
    data.m_amplitude = Functions::DecibelToAmplitude(volumeDb);
    m_commands->Add(cmd);
}

float rf::Send::GetVolumeDb() const
{
    return Functions::AmplitudeToDecibel(m_amplitude);
}

rf::MixGroupHandle rf::Send::GetSendToMixGroupHandle() const
{
    return m_sendToMixGroupHandle;
}