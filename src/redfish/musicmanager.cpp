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

#include "musicmanager.h"

#include "audiotimeline.h"

rf::MusicManager::MusicManager(AudioTimeline* timeline, const AudioSpec& spec)
    : m_timeline(timeline)
    , m_musicDatabase(Allocator::Allocate<MusicDatabase>("MusicDatabase"))
    , m_conductor(m_musicDatabase, spec, &timeline->m_messenger)
    , m_sequencer(m_musicDatabase, spec, &timeline->m_messenger)
    , m_cuesToDestory(RF_MAX_CUES)
    , m_stingersToDestory(RF_MAX_STINGERS)
    , m_transitionsToDestory(RF_MAX_TRANSITIONS)
{
}

rf::MusicManager::~MusicManager()
{
    Allocator::Deallocate<MusicDatabase>(&m_musicDatabase);
}

void rf::MusicManager::Process(long long playhead, MixItem* outMixItems, int* outStartIndex)
{
    // Process Sequencer
    outMixItems[*outStartIndex].ZeroOut();
    const Sequencer::Result result = m_sequencer.Process(&m_conductor, playhead, outMixItems, outStartIndex, m_timeline->m_audioDataReferences);
    const MusicTransitionRequest& currentTransition = m_sequencer.GetCurrentTransition();
    const bool isPlaying = m_sequencer.IsPlaying();
    m_conductor.Update(playhead, currentTransition, isPlaying);

    // Process Stops
    if (result == Sequencer::Result::Stop)
    {
        m_conductor.Reset();
    }

    DeleteFromMusicDatabase();
}

void rf::MusicManager::Stop(long long stopTime, long long playhead)
{
    m_sequencer.Stop(stopTime, playhead);
}

void rf::MusicManager::Fade(long long startTime, float amplitude, int sampleDuration, long long playhead, bool stopOnDone)
{
    m_sequencer.Fade(startTime, amplitude, sampleDuration, playhead, stopOnDone);
}

void rf::MusicManager::AddTransition(int transitionIndex)
{
    m_sequencer.AddTransition(transitionIndex);
}

void rf::MusicManager::Unload(AudioHandle audioHandle, long long playhead)
{
    m_sequencer.Unload(audioHandle, playhead);
}

rf::MusicDatabase* rf::MusicManager::GetMusicDatabase()
{
    return m_musicDatabase;
}

const rf::MusicDatabase* rf::MusicManager::GetMusicDatabase() const
{
    return m_musicDatabase;
}

long long rf::MusicManager::CalculateStartTime(const Sync& sync, long long playhead) const
{
    const bool syncToMusic = m_sequencer.IsPlaying() || m_sequencer.IsProcessingTransition();
    return m_conductor.CalculateStartTime(sync, playhead, syncToMusic);
}

int rf::MusicManager::GetSyncSamples(const Sync& sync) const
{
    return m_conductor.GetSyncSamples(sync);
}

bool rf::MusicManager::IsPlaying() const
{
    return m_sequencer.IsPlaying();
}

rf::CueHandle rf::MusicManager::GetCurrentlyPlayingCueHandle() const
{
    const MusicTransitionRequest& currentTransition = m_sequencer.GetCurrentTransition();
    if (currentTransition.m_transitionDataIndex >= 0)
    {
        const MusicDatabase::TransitionData& transitionData = m_musicDatabase->GetTransitionData(currentTransition.m_transitionDataIndex);
        return m_musicDatabase->GetCueData(transitionData.m_cueIndex).m_cueHandle;
    }

    return CueHandle();
}

void rf::MusicManager::PickAndAddTransition(const void* userData)
{
    const Metronome& metronome = m_conductor.GetMetronome();
    const int currentBar = metronome.GetBarCounter();
    const int currentBeat = metronome.GetBeatCounter();
    const CueHandle cueHandle = GetCurrentlyPlayingCueHandle();

    const int index = m_musicDatabase->GetTransitionIndexThatMeetsCondition(userData, cueHandle, currentBar, currentBeat);
    if (index == -1)
    {
        return;
    }

    AddTransition(index);
}

void rf::MusicManager::DestroyCue(int cueIndex, long long playhead)
{
    m_sequencer.DestroyCue(cueIndex, playhead);
    m_cuesToDestory.Append(cueIndex);
}

void rf::MusicManager::DestroyTransition(int transitionIndex, long long playhead)
{
    m_sequencer.DestroyTransition(transitionIndex, playhead);
    m_transitionsToDestory.Append(transitionIndex);
}

void rf::MusicManager::DestroyStinger(int stingerIndex, long long playhead)
{
    m_sequencer.DestroyStinger(stingerIndex, playhead);
    m_stingersToDestory.Append(stingerIndex);
}

void rf::MusicManager::DeleteFromMusicDatabase()
{
    Messenger* messenger = &m_timeline->m_messenger;

    for (int index : m_cuesToDestory)
    {
        m_musicDatabase->DestroyCue(index, messenger);
    }

    for (int index : m_stingersToDestory)
    {
        m_musicDatabase->DestroyStinger(index, messenger);
    }

    for (int index : m_transitionsToDestory)
    {
        m_musicDatabase->DestroyTransition(index, messenger);
    }

    m_cuesToDestory.Clear();
    m_stingersToDestory.Clear();
    m_transitionsToDestory.Clear();
}
