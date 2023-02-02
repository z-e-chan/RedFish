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

#include "musicdatabase.h"

#include "allocator.h"
#include "assert.h"
#include "cue.h"
#include "messenger.h"
#include "stinger.h"
#include "transition.h"

rf::MusicDatabase::MusicDatabase()
{
    m_cueData = Allocator::AllocateArray<CueData>("MusicDatabaseCueData", RF_MAX_CUES);
    m_transitionData = Allocator::AllocateArray<TransitionData>("MusicDatabaseTransitionData", RF_MAX_TRANSITIONS);
    m_transitionHandles = Allocator::AllocateArray<TransitionHandle>("MusicDatabaseTransitionHandle", RF_MAX_TRANSITIONS);
    m_transitionConditions = Allocator::AllocateArray<TransitionCondition>("MusicDatabaseTransitionConditions", RF_MAX_TRANSITIONS);
    m_stingerData = Allocator::AllocateArray<StingerData>("MusicDatabaseStingerData", RF_MAX_STINGERS);
}

rf::MusicDatabase::~MusicDatabase()
{
    Allocator::DeallocateArray<CueData>(&m_cueData, RF_MAX_CUES);
    Allocator::DeallocateArray<TransitionData>(&m_transitionData, RF_MAX_TRANSITIONS);
    Allocator::DeallocateArray<TransitionHandle>(&m_transitionHandles, RF_MAX_TRANSITIONS);
    Allocator::DeallocateArray<TransitionCondition>(&m_transitionConditions, RF_MAX_TRANSITIONS);
    Allocator::DeallocateArray<StingerData>(&m_stingerData, RF_MAX_STINGERS);
}

void rf::MusicDatabase::CreateCue(const Cue* cue, int index)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_CUES, "Index out of bounds");
    CueData& data = m_cueData[index];
    data.m_cueHandle = cue->GetCueHandle();
    data.m_meter = cue->GetMeter();
    data.m_tempo = cue->GetTempo();
    data.m_gainDb = cue->GetGainDb();
    data.m_numLayers = cue->GetNumLayers();
    for (int i = 0; i < data.m_numLayers; ++i)
    {
        data.m_layers[i] = cue->GetLayer(i);
    }
}

void rf::MusicDatabase::DestroyCue(int index, Messenger* messenger)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_CUES, "Index out of bounds");
    m_cueData[index] = CueData();

    Message msg;
    msg.m_type = MessageType::MusicDestroyCue;
    msg.GetMusicDestroyCueData()->m_index = index;
    messenger->AddMessage(msg);
}

const rf::MusicDatabase::CueData& rf::MusicDatabase::GetCueData(int index) const
{
    RF_ASSERT(index >= 0 && index < RF_MAX_CUES, "Index out of bounds");
    return m_cueData[index];
}

void rf::MusicDatabase::CreateTransition(const Transition* transition, int index)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_TRANSITIONS, "Index out of bounds");
    TransitionData& data = m_transitionData[index];
    data.m_transitionHandle = transition->GetTransitionHandle();
    data.m_sync = transition->GetSync();
    data.m_playCount = transition->GetPlayCount();
    data.m_isLooping = transition->IsLooping();
    data.m_cueIndex = GetCueDataIndexByHandle(transition->GetCue()->GetCueHandle());

    m_transitionHandles[index] = transition->GetTransitionHandle();
    m_transitionConditions[index] = transition->GetTransitionCondition();

    if (const Transition* followUp = transition->GetFollowUpTransition())
    {
        for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
        {
            if (m_transitionData[i].m_transitionHandle == followUp->GetTransitionHandle())
            {
                data.m_followUpTransitionIndex = i;
                break;
            }
        }

        RF_ASSERT(data.m_followUpTransitionIndex >= 0, "Could not find follow up transition");
    }

    if (const Stinger* stinger = transition->GetStinger())
    {
        for (int i = 0; i < RF_MAX_STINGERS; ++i)
        {
            if (m_stingerData[i].m_stingerHandle == stinger->GetStingerHandle())
            {
                data.m_stingerIndex = i;
                break;
            }
        }

        RF_ASSERT(data.m_stingerIndex >= 0, "Could not find stinger");
    }
}

void rf::MusicDatabase::DestroyTransition(int index, Messenger* messenger)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_TRANSITIONS, "Index out of bounds");
    m_transitionData[index] = TransitionData();

    Message msg;
    msg.m_type = MessageType::MusicDestroyTransition;
    msg.GetMusicDestroyTransitionData()->m_index = index;
    messenger->AddMessage(msg);
}

const rf::MusicDatabase::TransitionData& rf::MusicDatabase::GetTransitionData(int index) const
{
    RF_ASSERT(index >= 0 && index < RF_MAX_TRANSITIONS, "Index out of bounds");
    return m_transitionData[index];
}

int rf::MusicDatabase::GetTransitionIndexThatMeetsCondition(const void* userData, CueHandle cueHandle, int currentBar, int currentBeat) const
{
    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        const TransitionHandle transitionHandle = m_transitionHandles[i];
        if (!transitionHandle)
        {
            continue;
        }

        if (m_transitionConditions[i].Matches(userData, cueHandle, currentBar, currentBeat))
        {
            return i;
        }
    }

    return -1;
}

void rf::MusicDatabase::CreateStinger(const Stinger* stinger, int index)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_STINGERS, "Index out of bounds");
    StingerData& data = m_stingerData[index];
    data.m_stingerHandle = stinger->GetStingerHandle();
    data.m_sync = stinger->GetSync();
    data.m_gainDb = stinger->GetGainDb();
    for (int i = 0; i < RF_MAX_CUES; ++i)
    {
        const CueHandle cueHandle = stinger->GetCueHandle();
        if (m_cueData[i].m_cueHandle == cueHandle)
        {
            data.m_cueIndex = i;
            break;
        }
    }

    RF_ASSERT(data.m_cueIndex >= 0, "Could not find cue for stinger");
}

void rf::MusicDatabase::DestroyStinger(int index, Messenger* messenger)
{
    RF_ASSERT(index >= 0 && index < RF_MAX_STINGERS, "Index out of bounds");
    m_stingerData[index] = StingerData();

    Message msg;
    msg.m_type = MessageType::MusicDestroyStinger;
    msg.GetMusicDestroyStingerData()->m_index = index;
    messenger->AddMessage(msg);
}

const rf::MusicDatabase::StingerData& rf::MusicDatabase::GetStingerData(int index) const
{
    RF_ASSERT(index >= 0 && index < RF_MAX_STINGERS, "Index out of bounds");
    return m_stingerData[index];
}

int rf::MusicDatabase::GetCueDataIndexByHandle(CueHandle cueHandle) const
{
    for (int i = 0; i < RF_MAX_CUES; ++i)
    {
        if (m_cueData[i].m_cueHandle == cueHandle)
        {
            return i;
        }
    }

    RF_FAIL("Could not find index for Cue");
    return -1;
}
