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

#include "conductor.h"

#include <cmath>

#include "assert.h"
#include "audiodata.h"
#include "functions.h"
#include "musicdatabase.h"

rf::Conductor::Conductor(MusicDatabase* musicDatabase, const AudioSpec& spec, Messenger* messanger)
    : m_spec(spec)
    , m_metronome(spec, musicDatabase, messanger)
    , m_musicDatabase(musicDatabase)
{
}

void rf::Conductor::Update(long long playhead, const MusicTransitionRequest& currentRequest, bool isPlaying)
{
    m_metronome.Update(playhead, currentRequest, isPlaying);
}

void rf::Conductor::Reset()
{
    m_lastCreatedRequest = MusicTransitionRequest();
    m_metronome.Reset();
}

rf::MusicTransitionRequest rf::Conductor::CreateRequest(int transitionIndex, long long playhead, bool isPlaying, const AudioData** audioData)
{
    MusicTransitionRequest request;
    request.m_transitionDataIndex = transitionIndex;

    const MusicDatabase::TransitionData& transitionData = m_musicDatabase->GetTransitionData(transitionIndex);
    const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(transitionData.m_cueIndex);

    // If there is no music playing when this function is called we have no tempo or metre set.
    // This means that we cannot reliably call m_metronome.GetSyncSamples() for stingerDuration.
    // Therefore, we must set the metronome so we can get proper timings.
    if (!isPlaying)
    {
        RF_ASSERT(Functions::FloatEquality(-1.0f, m_metronome.GetTempo()), "Expected -1.0f tempo");
        RF_ASSERT(m_metronome.GetMeter() == Meter(), "Expected null metre");
        m_metronome.Set(cueData.m_tempo, cueData.m_meter);
    }

    RF_ASSERT(cueData.m_numLayers > 0, "Expected cue to have layers");
    const int layerAudioDataIndex = cueData.m_layers[0].m_audioDataIndex;
    const int musicLength = audioData[layerAudioDataIndex]->m_numFrames;
    RF_ASSERT(musicLength > 0, "m_numFrames shoud be greater than 0, is there an asset issue?");

    long long transitionStartTime = CalculateStartTime(transitionData.m_sync, playhead, isPlaying);
    RF_ASSERT(transitionStartTime > 0, "Music cannout start at time 0");

    if (transitionData.m_stingerIndex >= 0)
    {
        const MusicDatabase::StingerData& stingerData = m_musicDatabase->GetStingerData(transitionData.m_stingerIndex);
        const MusicDatabase::CueData& stingerCueData = m_musicDatabase->GetCueData(stingerData.m_cueIndex);

        const int stingerDuration = Metronome::GetSyncSamples(m_spec, stingerData.m_sync, stingerCueData.m_tempo, stingerCueData.m_meter);

        // Not enough time to play the stinger.
        if (transitionStartTime - stingerDuration < playhead)
        {
            // Extend the transition time further in time to accommodate the stinger.
            int extension = -1;

            // If the transition is queue, we have to extend by the music length.
            if (transitionData.m_sync.m_sync == Sync::Value::Queue)
            {
                extension = musicLength;
            }
            // If the transition is not a queue, we extend by the sync time.
            else
            {
                // We need to put an extension in the music transition because we don't have enough time to play the
                // stinger. We need to extend transitionStartTime. transitionStartTime is created by
                // CalculateStartTime() which does math with m_metronome's data. Therefore, when we extend it, we want
                // to use m_metronome's data to create the correct extension.
                extension = Metronome::GetSyncSamples(m_spec, transitionData.m_sync, m_metronome.GetTempo(), m_metronome.GetMeter());
            }

            // While the stinger does not fit, we increment by the extension.
            while (transitionStartTime - stingerDuration < playhead)
            {
                transitionStartTime += extension;
            }
        }

        request.m_stingerStartTime = transitionStartTime - stingerDuration;
    }

    RF_ASSERT(transitionStartTime > 0, "Music cannout start at time 0");
    request.m_startTime = transitionStartTime;
    request.m_musicLength = musicLength;

    // If we are playing do not update the request here.
    // The issue is that request will get replaced much too early, before the transition actually happens.
    // This will cause an inconsistency with the start time of the current request.
    // The start time will be what we 'just' queued up, but that does not agree with what we hear playing.
    // The result will be an incorrect music sync if we were then to play a stinger.
    // We call UpdateLastCreatedRequest during the first window of the transition in Sequencer.
    if (!isPlaying)
    {
        UpdateLastCreatedRequest(request);
    }

    return request;
}

long long rf::Conductor::CalculateStartTime(const Sync& sync, long long playhead, bool isPlaying) const
{
    switch (sync.m_mode)
    {
        case Sync::Mode::Musical:
        {
            long long startTime = playhead;
            if (isPlaying)
            {
                RF_ASSERT(m_lastCreatedRequest.m_startTime >= 0,
                          "We should have a start time at this point because we were told something is playing");
                if (sync.m_sync == Sync::Value::Queue)
                {
                    startTime = m_lastCreatedRequest.m_startTime + m_lastCreatedRequest.m_musicLength;
                }
                else
                {
                    // If we are dealing with a Cut transition, all we need to do is return the playhead.
                    // Cuts happen ASAP and nothing is faster than the playhead.
                    if (sync.m_sync == Sync::Value::Cut)
                    {
                        return playhead;
                    }

                    const long long syncValue = static_cast<long long>(m_metronome.PreciseBeatSwitch(sync));

                    // Calculate the start transition wrt the start of the cue.
                    if (sync.m_referencePoint == Sync::ReferencePoint::CueStart)
                    {
                        startTime = m_lastCreatedRequest.m_startTime + syncValue;

                        // startTime needs to be >= playhead
                        if (startTime >= playhead)
                        {
                            return startTime;
                        }

                        RF_FAIL(
                            "The result of a Sync::ReferencePoint::CueStart had the transition occur some time before the playhead. This is a result of a poorly authored music transition. I am going to use a Sync::ReferencePoint::Bar now instead.");
                    }

                    // We perform musical transition wrt the last played bar.
                    // If we ask for '1-bar' we play at the next bar.
                    // If we ask for '7/8', we get the first '7/8' interval in the current measure.
                    // If we are past '7/8' in the current measure, we will get '7/8' in the next measure.

                    const int numFullBarsPlayed = m_metronome.GetBarCounter() - 1;
                    const long long barValue = static_cast<long long>(m_metronome.PreciseBeatSwitch(Sync(Sync::Value::Bar)));
                    const long long currentMeasureStartTime = m_lastCreatedRequest.m_startTime + (numFullBarsPlayed * barValue);
                    startTime = -1;

                    long long referencePoint = currentMeasureStartTime;
                    long long nextMeasure = currentMeasureStartTime + barValue;
                    int counter = 0;

                    while (startTime < playhead)
                    {
                        RF_ASSERT(
                            counter < 1000,
                            "Expected to find a transition in less than 1000 iterations. Did we forget to call UpdateLastCreatedRequest somewhere?");

                        startTime = referencePoint + syncValue;

                        referencePoint += syncValue;
                        if (referencePoint >= nextMeasure)
                        {
                            referencePoint = nextMeasure;
                            nextMeasure += barValue;
                        }

                        ++counter;
                    }
                }
            }
            return startTime;
        }
        case Sync::Mode::Time:
        {
            return playhead + static_cast<long long>(round(m_spec.m_sampleRate * sync.m_time));
        }
        default: return playhead;
    }
}

int rf::Conductor::GetSyncSamples(const Sync& sync) const
{
    return Metronome::GetSyncSamples(m_spec, sync, m_metronome.GetTempo(), m_metronome.GetMeter());
}

void rf::Conductor::UpdateLastCreatedRequest(const MusicTransitionRequest& request)
{
    m_lastCreatedRequest = request;
}

const rf::Metronome& rf::Conductor::GetMetronome() const
{
    return m_metronome;
}