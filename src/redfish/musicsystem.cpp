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

#include "musicsystem.h"

#include "allocator.h"
#include "assert.h"
#include "commandprocessor.h"
#include "cue.h"
#include "message.h"
#include "musiccommands.h"
#include "stinger.h"
#include "transition.h"

rf::MusicSystem::MusicSystem(CommandProcessor* commands, AssetSystem* assetSystem)
    : m_commands(commands)
    , m_assetSystem(assetSystem)
{
    m_cues = Allocator::AllocateArray<Cue>("MusicSystemCues", RF_MAX_CUES, CueParameters(), nullptr, -1);
    m_transitions = Allocator::AllocateArray<Transition>("MusicSystemTransitions", RF_MAX_TRANSITIONS, TransitionParameters(), -1);
    m_stingers = Allocator::AllocateArray<Stinger>("MusicSystemStingers", RF_MAX_STINGERS, StingerParameters(), -1);
}

rf::MusicSystem::~MusicSystem()
{
    Allocator::DeallocateArray<Cue>(&m_cues, RF_MAX_CUES);
    Allocator::DeallocateArray<Transition>(&m_transitions, RF_MAX_TRANSITIONS);
    Allocator::DeallocateArray<Stinger>(&m_stingers, RF_MAX_STINGERS);
}

rf::Cue* rf::MusicSystem::CreateCue(const CueParameters& parameters)
{
    for (int i = 0; i < RF_MAX_CUES; ++i)
    {
        if (!m_cues[i])
        {
            RF_ASSERT(parameters.m_numLayers > 0, "Expected layers");

            Cue* cue = new (m_cues + i) Cue(parameters, m_assetSystem, i);

            AudioCommand cmd;
            CreateCueCommand& data = EncodeAudioCommand<CreateCueCommand>(&cmd);
            data.m_index = i;
            data.m_cue = cue;
            m_commands->Add(cmd);

            return cue;
        }
    }

    RF_FAIL("Could not create Cue. Increase RF_MAX_CUES");
    return nullptr;
}

void rf::MusicSystem::DestroyCue(Cue** cue)
{
    if (!(*cue))
    {
        return;
    }

    if ((*cue)->GetCueHandle() == m_currentCueHandle)
    {
        m_currentCueHandle = CueHandle();
    }

    const CueHandle cueHandle = (*cue)->GetCueHandle();
    for (int i = 0; i < RF_MAX_CUES; ++i)
    {
        if (m_cues[i].GetCueHandle() == cueHandle)
        {
            AudioCommand cmd;
            DestroyCueCommand& data = EncodeAudioCommand<DestroyCueCommand>(&cmd);
            data.m_index = i;
            m_commands->Add(cmd);

            *cue = nullptr;
            return;
        }
    }

    RF_FAIL("Could not find cue to destroy");
}

rf::Transition* rf::MusicSystem::CreateTransition(const TransitionParameters& parameters)
{
    RF_ASSERT(parameters.m_cue, "Expected a valid cue");

    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        if (!m_transitions[i])
        {
            RF_ASSERT(parameters.m_cue, "Expect a valid cue");

            Transition* transition = new (m_transitions + i) Transition(parameters, i);

            AudioCommand cmd;
            CreateTransitionCommand& data = EncodeAudioCommand<CreateTransitionCommand>(&cmd);
            data.m_index = i;
            data.m_transition = transition;
            m_commands->Add(cmd);

            return transition;
        }
    }

    RF_FAIL("Could not create Transition. Increase RF_MAX_TRANSITIONS");
    return nullptr;
}

void rf::MusicSystem::DestroyTransition(Transition** transition)
{
    if (!(*transition))
    {
        return;
    }

    const TransitionHandle transitionHandle = (*transition)->GetTransitionHandle();
    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        if (m_transitions[i].GetTransitionHandle() == transitionHandle)
        {
            AudioCommand cmd;
            DestroyTransitionCommand& data = EncodeAudioCommand<DestroyTransitionCommand>(&cmd);
            data.m_index = i;
            m_commands->Add(cmd);

            *transition = nullptr;
            return;
        }
    }

    RF_FAIL("Could not find cue to destroy");
}

rf::Stinger* rf::MusicSystem::CreateStinger(const StingerParameters& parameters)
{
    for (int i = 0; i < RF_MAX_STINGERS; ++i)
    {
        if (!m_stingers[i])
        {
            RF_ASSERT(parameters.m_cue, "Expect a valid cue");

            Stinger* stinger = new (m_stingers + i) Stinger(parameters, i);

            AudioCommand cmd;
            CreateStingerCommand& data = EncodeAudioCommand<CreateStingerCommand>(&cmd);
            data.m_index = i;
            data.m_stinger = stinger;
            m_commands->Add(cmd);

            return stinger;
        }
    }

    RF_FAIL("Could not create Stinger. Increase RF_MAX_STINGERS");
    return nullptr;
}

void rf::MusicSystem::DestroyStinger(Stinger** stinger)
{
    if (!(*stinger))
    {
        return;
    }

    const StingerHandle stingerHandle = (*stinger)->GetStingerHandle();
    for (int i = 0; i < RF_MAX_STINGERS; ++i)
    {
        if (m_stingers[i].GetStingerHandle() == stingerHandle)
        {
            AudioCommand cmd;
            DestroyStingerCommand& data = EncodeAudioCommand<DestroyStingerCommand>(&cmd);
            data.m_index = i;
            m_commands->Add(cmd);

            *stinger = nullptr;
            return;
        }
    }

    RF_FAIL("Could not find cue to destroy");
}

void rf::MusicSystem::Play(const Transition* transition) const
{
    RF_ASSERT(transition, "Expected a valid transition");
    RF_ASSERT(transition->GetIndex() >= 0, "Expected index to be set");
    AudioCommand cmd;
    PlayTransitionCommand& data = EncodeAudioCommand<PlayTransitionCommand>(&cmd);
    data.m_index = transition->GetIndex();
    m_commands->Add(cmd);
}

void rf::MusicSystem::Play(UserData userData) const
{
    AudioCommand cmd;
    PickTransitionCommand& data = EncodeAudioCommand<PickTransitionCommand>(&cmd);
    memcpy(data.m_userData, userData.m_data, TransitionCondition::k_dataSize);
    m_commands->Add(cmd);
}

void rf::MusicSystem::Play(const Stinger* stinger) const
{
    RF_ASSERT(stinger, "Expected a valid stinger");
    AudioCommand cmd;
    PlayStingerCommand& data = EncodeAudioCommand<PlayStingerCommand>(&cmd);
    data.m_index = stinger->GetIndex();
    m_commands->Add(cmd);
}

void rf::MusicSystem::Stop() const
{
    AudioCommand cmd;
    StopMusicCommand& data = EncodeAudioCommand<StopMusicCommand>(&cmd);
    m_commands->Add(cmd);
}

void rf::MusicSystem::FadeOutAndStop(const Sync& sync, const Sync& duration) const
{
    RF_ASSERT(sync.m_sync != Sync::Value::Queue, "Fades do not support Queue");
    RF_ASSERT(duration.m_sync != Sync::Value::Queue, "Fades do not support Queue");

    AudioCommand cmd;
    FadeOutAndStopMusicCommand& data = EncodeAudioCommand<FadeOutAndStopMusicCommand>(&cmd);
    data.m_sync = sync;
    data.m_duration = duration;
    m_commands->Add(cmd);
}

rf::CueHandle rf::MusicSystem::GetCurrentCueHandle() const
{
    return m_currentCueHandle;
}

const char* rf::MusicSystem::GetCurrentCueName() const
{
    if (!m_currentCueHandle)
    {
        return "";
    }

    for (int i = 0; i < RF_MAX_CUES; ++i)
    {
        if (m_cues[i].GetCueHandle() == m_currentCueHandle)
        {
            return m_cues[i].GetName();
        }
    }

    RF_FAIL("Could not find cue");
    return "";
}

const rf::Meter& rf::MusicSystem::GetCurrentMeter() const
{
    return m_meter;
}

int rf::MusicSystem::GetCurrentBar() const
{
    return m_bar;
}

int rf::MusicSystem::GetCurrentBeat() const
{
    return m_beat;
}

float rf::MusicSystem::GetCurrentTempo() const
{
    return m_tempo;
}

bool rf::MusicSystem::ProcessMessages(const Message& message)
{
    switch (message.m_type)
    {
        case MessageType::MusicDestroyCue:
        {
            const int index = message.GetMusicDestroyCueData()->m_index;
            memset(m_cues + index, 0, sizeof(Cue));
            return true;
        }
        case MessageType::MusicDestroyTransition:
        {
            const int index = message.GetMusicDestroyTransitionData()->m_index;
            memset(m_transitions + index, 0, sizeof(Transition));
            return true;
        }
        case MessageType::MusicDestroyStinger:
        {
            const int index = message.GetMusicDestroyStingerData()->m_index;
            memset(m_stingers + index, 0, sizeof(Stinger));
            return true;
        }
        case MessageType::MusicCurrentBar:
        {
            const Message::MusicCurrentBarData* data = message.GetMusicCurrentBarData();
            m_bar = data->m_currentBar;
            return true;
        }
        case MessageType::MusicCurrentBeat:
        {
            const Message::MusicCurrentBeatData* data = message.GetMusicCurrentBeatData();
            m_beat = data->m_currentBeat;
            return true;
        }
        case MessageType::MusicBarChanged:
        {
            const Message::MusicBarChangedData* data = message.GetMusicBarChangedData();
            data->m_currentBar;
            data->m_currentBeat;
            return true;
        }
        case MessageType::MusicBeatChanged:
        {
            const Message::MusicBeatChangedData* data = message.GetMusicBeatChangedData();
            data->m_currentBar;
            data->m_currentBeat;
            return true;
        }
        case MessageType::MusicTempo:
        {
            const Message::MusicTempoData* data = message.GetMusicTempoData();
            m_tempo = data->m_tempo;
            return true;
        }
        case MessageType::MusicMeter:
        {
            const Message::MusicMeterData* data = message.GetMusicMeterData();
            m_meter.m_top = data->m_top;
            m_meter.m_bottom = data->m_bottom;
            return true;
        }
        case MessageType::MusicTransitioned:
        {
            const Message::MusicTransitionedData* data = message.GetMusicTransitionedData();
            m_currentCueHandle = data->m_toCueHandle;
            return true;
        }
        case MessageType::MusicFinished:
        {
            m_currentCueHandle = CueHandle();
            m_meter = Meter();
            m_bar = 0;
            m_beat = 0;
            m_tempo = 0.0f;
            return true;
        }
        default: return false;
    }
}