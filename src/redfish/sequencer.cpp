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

#include "sequencer.h"

#include "assert.h"
#include "audiodata.h"
#include "conductor.h"
#include "functions.h"
#include "messenger.h"

rf::Sequencer::Sequencer(const MusicDatabase* musicDatabase, const AudioSpec& spec, Messenger* messanger)
    : m_spec(spec)
    , m_musicDatabase(musicDatabase)
    , m_messanger(messanger)
    , m_fader(spec.m_bufferSize)
    , m_transformationMixItem(spec.m_channels, spec.m_bufferSize)
    , m_layerSet(spec, messanger)
    , m_stingerSet(spec, messanger)
{
    Reset(true);
}

rf::Sequencer::Result rf::Sequencer::Process(Conductor* conductor,
                                             long long playhead,
                                             MixItem* outMixItems,
                                             int* outstartIndex,
                                             const AudioData** audioData)
{
    int mixItemStartIndex = *outstartIndex;
    const int bufferSize = m_spec.m_bufferSize;
    BaseVoice::Info info;
    bool process = false;

    do
    {
        bool nothingToDo = false;
        bool foundTransition = false;

        switch (m_state)
        {
            case State::GetTransition:
            {
                if (GetRequest(playhead, conductor, &m_pendingTransition, audioData))
                {
                    foundTransition = true;
                    m_state = State::ProcessingTransition;

                    const int pendingTransitionIndex = m_pendingTransition.m_transitionDataIndex;
                    const MusicDatabase::TransitionData& transitionData = m_musicDatabase->GetTransitionData(pendingTransitionIndex);
                    if (transitionData.m_stingerIndex >= 0)
                    {
                        const MusicDatabase::StingerData& stingerData = m_musicDatabase->GetStingerData(transitionData.m_stingerIndex);
                        const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(stingerData.m_cueIndex);
                        m_stingerSet.Play(m_pendingTransition, stingerData, cueData, audioData);
                    }
                }

                if (!foundTransition)
                {
                    break;
                }
            }
            case State::ProcessingTransition:
            {
                if (Functions::InFirstWindow(playhead, m_pendingTransition.m_startTime, m_spec.m_bufferSize))
                {
                    // We are in the first frame of the transition.

                    // There was some music playing before we got here. Therefore, that music was transitioned from.
                    // In other words, it was interrupted.
                    // This means that we need to send a voice stop message letting the engine know that the voices
                    // associated with the last music has stopped playing. Since base voice does not have a reliable way
                    // of knowing when a voice has been transitioned from, we need to inform it with this bool.

                    const int currentTransitionDataIndex = m_currentTransition.m_transitionDataIndex;
                    const bool currentTransitionIsValid = currentTransitionDataIndex >= 0;
                    const bool previousMusicWasInterrupted = currentTransitionIsValid;

                    CueHandle lastCueHandle;
                    if (currentTransitionIsValid)
                    {
                        const MusicDatabase::TransitionData& transitionData = m_musicDatabase->GetTransitionData(currentTransitionDataIndex);
                        const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(transitionData.m_cueIndex);
                        lastCueHandle = cueData.m_cueHandle;
                    }

                    m_currentTransition = m_pendingTransition;
                    m_pendingTransition = MusicTransitionRequest();
                    m_state = State::GetTransition;

                    const int currentTransitionIndex = m_currentTransition.m_transitionDataIndex;
                    RF_ASSERT(currentTransitionIndex >= 0, "We expected our new transition to be valid");
                    const MusicDatabase::TransitionData& transitionData = m_musicDatabase->GetTransitionData(currentTransitionIndex);
                    const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(transitionData.m_cueIndex);
                    const CueHandle currentCueHandle = cueData.m_cueHandle;

                    // We are transitioning.
                    // Figure out how many samples we need to take from the previous music cue.
                    // Start from index 0 and only process those samples.
                    const long long difference = m_currentTransition.m_startTime - playhead;
                    RF_ASSERT(difference <= bufferSize, "Expected this value to be <= bufferSize");
                    const int processLength = static_cast<int>(difference);
                    info = m_layerSet.Process(playhead, 0, processLength, previousMusicWasInterrupted, outMixItems, outstartIndex);
                    RF_ASSERT(!info.m_mixItemFullyFilled, "This mix item should not be filled at this point");
                    RF_ASSERT(info.m_lastFilledFrame < m_spec.m_bufferSize - 1, "This should have never filled up to this index");

                    // Play the new transition
                    m_layerSet.Play(m_currentTransition, transitionData, cueData, audioData);
                    conductor->UpdateLastCreatedRequest(m_currentTransition);

                    Message musicTransitionedMsg;
                    musicTransitionedMsg.m_type = MessageType::MusicTransitioned;
                    Message::MusicTransitionedData* data = musicTransitionedMsg.GetMusicTransitionedData();
                    data->m_toCueHandle = currentCueHandle;
                    data->m_fromCueHandle = lastCueHandle;
                    m_messanger->AddMessage(musicTransitionedMsg);
                }
                else
                {
                    // If we were in the GetTransition state and we found a transition with delay (a stinger or
                    // time delay) and we were not playing any music prior, we would get stuck in an infinite loop. The
                    // problem would be that we set foundTransition to true but Functions::InFirstWindow
                    // would fail due to the delay. Since we are not playing anything, info.m_mixItemFullyFilled would
                    // be false. As such, we would never set process to false and we would be stuck. The solution is
                    // have a nothingToDo bool that can let us opt in to not processing when know better.
                    nothingToDo = true;
                }
                break;
            }
            case State::FollowUp:
            {
                m_state = m_goToAfterFollowUp;

                const int currentTransitionDataIndex = m_currentTransition.m_transitionDataIndex;
                const MusicDatabase::TransitionData& transitionData = m_musicDatabase->GetTransitionData(currentTransitionDataIndex);
                const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(transitionData.m_cueIndex);
                const CueHandle lastCueHandle = cueData.m_cueHandle;

                const MusicDatabase::TransitionData& followUpTransitionData =
                    m_musicDatabase->GetTransitionData(transitionData.m_followUpTransitionIndex);
                const MusicDatabase::CueData& followUpCue = m_musicDatabase->GetCueData(followUpTransitionData.m_cueIndex);
                RF_ASSERT(followUpCue.m_numLayers > 0, "Expected follow up to have layers");
                const int followUpAudioIndex = followUpCue.m_layers[0].m_audioDataIndex;

                MusicTransitionRequest followUpRequest;
                followUpRequest.m_startTime = playhead + info.m_lastFilledFrame + 1;
                followUpRequest.m_stingerStartTime = -1;
                followUpRequest.m_musicLength = audioData[followUpAudioIndex]->m_numFrames;
                followUpRequest.m_transitionDataIndex = transitionData.m_followUpTransitionIndex;
                m_currentTransition = followUpRequest;
                m_layerSet.Play(m_currentTransition, followUpTransitionData, followUpCue, audioData);
                conductor->UpdateLastCreatedRequest(m_currentTransition);

                RF_ASSERT(followUpCue.m_cueHandle, "We expected our new transition to be valid");
                Message musicTransitionedMsg;
                musicTransitionedMsg.m_type = MessageType::MusicTransitioned;
                Message::MusicTransitionedData* data = musicTransitionedMsg.GetMusicTransitionedData();
                data->m_toCueHandle = followUpCue.m_cueHandle;
                data->m_fromCueHandle = lastCueHandle;
                m_messanger->AddMessage(musicTransitionedMsg);
                break;
            }
            default: break;
        }

        const int startingIndex = info.m_lastFilledFrame + 1;
        const int fillLength = bufferSize - startingIndex;
        RF_ASSERT(startingIndex < bufferSize, "Starting index out of bounds");
        RF_ASSERT(fillLength <= bufferSize, "Fill length out of bounds");
        info = m_layerSet.Process(playhead, startingIndex, fillLength, false, outMixItems, outstartIndex);

        // Check for follow ups
        bool followUp = false;
        if (info.m_done)
        {
            const int currentTransitionIndex = m_currentTransition.m_transitionDataIndex;
            RF_ASSERT(currentTransitionIndex >= 0, "We transition to be valid");
            const MusicDatabase::TransitionData& transitionData = m_musicDatabase->GetTransitionData(currentTransitionIndex);
            if (transitionData.m_followUpTransitionIndex >= 0)
            {
                m_goToAfterFollowUp = m_state;
                m_state = State::FollowUp;
                followUp = true;
            }
        }

        if (foundTransition)
        {
            // We have found a transition.
            // We need to reprocess the window so that the transition can 'butt up' to whatever was playing last.
            process = true;
        }

        if (followUp)
        {
            // There is a follow up.
            // Reprocess so that the follow up can 'butt up' to whatever was playing last.
            process = true;
        }

        if (info.m_done && !followUp)
        {
            // We are done and there is no follow up.
            // No need to reprocess
            process = false;
        }

        if (info.m_mixItemFullyFilled)
        {
            // Have have filled every sample in this mix item.
            // There is no more work that can be done this window.
            process = false;
        }

        if (nothingToDo)
        {
            process = false;
        }
    } while (process);

    // Process our stingers(s)
    m_stingerSet.Process(playhead, bufferSize, outMixItems, outstartIndex);

    // The music has looped, so update the start time to reflect the loop.
    if (info.m_looped)
    {
        m_currentTransition.m_startTime += m_currentTransition.m_musicLength;

        // Update the conductor so that is is aware of the new m_startTime due to the loop.
        conductor->UpdateLastCreatedRequest(m_currentTransition);
    }

    // Process DSP
    const Result dspResult = UpdateDSP(outMixItems, mixItemStartIndex, *outstartIndex);

    // Determine if the music has stopped.
    // This can be caused by asking the music system to stop. Or, the music can end naturally.
    bool stop = false;
    bool naturalStop = false;
    if (dspResult == Result::Stop)
    {
        // The music system has been asked to stop.
        stop = true;
    }
    else
    {
        // The music we are playing is done and we have no more transitions to process.
        // We need to guard the case where info.m_done is true on the last frame of a window and there is a follow up to
        // do next callback. Despite info.m_done being true, we are not actually done because there is still a follow up
        // to do.
        const bool isHandlingFollowUpNextCallback = m_state == State::FollowUp;
        const bool isProcessingTransition = m_state == State::ProcessingTransition;
        const bool moreMusicToPlay = isHandlingFollowUpNextCallback || isProcessingTransition || m_numTransitions > 0;
        stop = info.m_done && !moreMusicToPlay;
        if (stop)
        {
            naturalStop = true;
        }
    }

    if (stop)
    {
        Message msg;
        msg.m_type = MessageType::MusicFinished;
        m_messanger->AddMessage(msg);

        info.m_stopped = true;

        conductor->Reset();

        // In the case of a natural stop, we do not want to reset the stingers. We want them to ring out.
        // In the case of a manual stop, we do want to reset the stingers.
        Reset(!naturalStop);
        return Result::Stop;
    }

    return Result::None;
}

void rf::Sequencer::Stop(long long stopTime, long long playhead)
{
    m_isStopping = true;
    m_fader.Update(0.0f, stopTime - playhead, k_stopSamples);
}

void rf::Sequencer::Fade(long long startTime, float amplitude, int sampleDuration, long long playhead, bool stopOnDone)
{
    m_fader.Update(amplitude, startTime - playhead, sampleDuration);
    m_stopOnDoneFade = stopOnDone;
}

void rf::Sequencer::Reset(bool resetStingers)
{
    m_pendingTransition = MusicTransitionRequest();
    m_currentTransition = MusicTransitionRequest();

    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        m_transitionIndices[i] = -1;
    }

    m_fader.Reset();
    m_transformationMixItem.Set(1.0f);
    m_transitionInsertIndex = 0;
    m_transitionGetIndex = 0;
    m_numTransitions = 0;
    m_state = State::GetTransition;
    m_layerSet.Reset();
    m_isStopping = false;
    m_stopOnDoneFade = false;

    if (resetStingers)
    {
        m_stingerSet.Reset();
    }
}

void rf::Sequencer::AddTransition(int transitionIndex)
{
    if (m_transitionIndices[m_transitionInsertIndex] == -1)
    {
        m_transitionIndices[m_transitionInsertIndex] = transitionIndex;
        m_transitionInsertIndex = (m_transitionInsertIndex + 1) % RF_MAX_TRANSITIONS;
        ++m_numTransitions;
    }
    else
    {
        RF_FAIL("Overriding music transition.");
    }
}

bool rf::Sequencer::IsPlaying() const
{
    return m_layerSet.IsPlaying();
}

bool rf::Sequencer::IsProcessingTransition() const
{
    return m_state == State::ProcessingTransition;
}

const rf::MusicTransitionRequest& rf::Sequencer::GetCurrentTransition() const
{
    return m_currentTransition;
}

bool rf::Sequencer::Unload(AudioHandle audioHandle, long long playhead)
{
    m_stingerSet.ResetIfPlayingAudioHandle(audioHandle);

    const auto CheckForAudioHandle = [this](int transitionIndex, AudioHandle audioHandle, long long playhead) -> bool {
        if (transitionIndex == -1)
        {
            return false;
        }

        const MusicDatabase::TransitionData& currentTransition = m_musicDatabase->GetTransitionData(transitionIndex);
        const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(currentTransition.m_cueIndex);

        for (int i = 0; i < cueData.m_numLayers; ++i)
        {
            if (cueData.m_layers[i].m_audioHandle == audioHandle)
            {
                Stop(playhead, playhead);
                return true;
            }
        }

        return false;
    };

    if (CheckForAudioHandle(m_currentTransition.m_transitionDataIndex, audioHandle, playhead))
    {
        return true;
    }

    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        if (CheckForAudioHandle(m_transitionIndices[i], audioHandle, playhead))
        {
            return true;
        }
    }

    return false;
}

void rf::Sequencer::DestroyCue(int cueIndex, long long playhead)
{
    const auto CheckForCueIndex = [this](int transitionIndex, int cueIndex, long long playhead) -> bool {
        if (transitionIndex == -1)
        {
            return false;
        }

        const MusicDatabase::TransitionData& currentTransition = m_musicDatabase->GetTransitionData(transitionIndex);
        const int transitionCueIndex = currentTransition.m_cueIndex;
        const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(transitionCueIndex);

        for (int i = 0; i < cueData.m_numLayers; ++i)
        {
            m_stingerSet.ResetIfPlayingAudioHandle(cueData.m_layers[i].m_audioHandle);
        }

        if (cueIndex == transitionCueIndex)
        {
            Stop(playhead, playhead);
            return true;
        }

        return false;
    };

    if (CheckForCueIndex(m_currentTransition.m_transitionDataIndex, cueIndex, playhead))
    {
        return;
    }

    if (CheckForCueIndex(m_pendingTransition.m_transitionDataIndex, cueIndex, playhead))
    {
        return;
    }

    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        if (CheckForCueIndex(m_transitionIndices[i], cueIndex, playhead))
        {
            return;
        }
    }
}

void rf::Sequencer::DestroyTransition(int transitionIndex, long long playhead)
{
    if (m_pendingTransition.m_transitionDataIndex == transitionIndex || m_currentTransition.m_transitionDataIndex == transitionIndex)
    {
        Stop(playhead, playhead);
        return;
    }

    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        if (m_transitionIndices[i] == transitionIndex)
        {
            Stop(playhead, playhead);
            return;
        }
    }
}

void rf::Sequencer::DestroyStinger(int stingerIndex, long long playhead)
{
    const auto CheckForStingerIndex = [this](int transitionIndex, int stingerIndex, long long playhead) -> bool {
        if (transitionIndex == -1)
        {
            return false;
        }

        const MusicDatabase::TransitionData& currentTransition = m_musicDatabase->GetTransitionData(transitionIndex);
        const int transitionStingerIndex = currentTransition.m_stingerIndex;

        if (transitionStingerIndex > 0)
        {
            const MusicDatabase::StingerData& stingerData = m_musicDatabase->GetStingerData(transitionStingerIndex);
            const MusicDatabase::CueData& cueData = m_musicDatabase->GetCueData(stingerData.m_cueIndex);

            for (int i = 0; i < cueData.m_numLayers; ++i)
            {
                m_stingerSet.ResetIfPlayingAudioHandle(cueData.m_layers[i].m_audioHandle);
            }
        }

        if (stingerIndex == transitionStingerIndex)
        {
            Stop(playhead, playhead);
            return true;
        }

        return false;
    };

    if (CheckForStingerIndex(m_currentTransition.m_transitionDataIndex, stingerIndex, playhead))
    {
        return;
    }

    if (CheckForStingerIndex(m_pendingTransition.m_transitionDataIndex, stingerIndex, playhead))
    {
        return;
    }

    for (int i = 0; i < RF_MAX_TRANSITIONS; ++i)
    {
        if (CheckForStingerIndex(m_transitionIndices[i], stingerIndex, playhead))
        {
            return;
        }
    }
}

bool rf::Sequencer::GetRequest(long long playhead, Conductor* conductor, MusicTransitionRequest* outRequest, const AudioData** audioData)
{
    if (m_state == State::ProcessingTransition)
    {
        return false;
    }

    if (m_numTransitions > 0)
    {
        const int transitionIndex = m_transitionIndices[m_transitionGetIndex];
        *outRequest = conductor->CreateRequest(transitionIndex, playhead, IsPlaying(), audioData);
        RF_ASSERT(outRequest->m_transitionDataIndex >= 0, "Expected valid transition");
        m_transitionIndices[m_transitionGetIndex] = -1;
        m_transitionGetIndex = (m_transitionGetIndex + 1) % RF_MAX_TRANSITIONS;
        --m_numTransitions;
        return true;
    }

    return false;
}

rf::Sequencer::Result rf::Sequencer::UpdateDSP(MixItem* outMixItems, int startIndex, int endIndex)
{
    m_transformationMixItem.Set(1.0f);

    const bool isFadingBefore = m_fader.IsFading();
    const bool isFading = m_fader.Process(&m_transformationMixItem, m_spec.m_bufferSize);

    for (int i = startIndex; i < endIndex; ++i)
    {
        outMixItems[i].Multiply(m_transformationMixItem);
    }

    const bool fadeIsComplete = !m_isStopping && isFadingBefore && !isFading;
    const bool stopOnDoneFade = fadeIsComplete && m_stopOnDoneFade;
    const bool stopIsComplete = !isFading && m_isStopping;
    const bool stop = stopIsComplete || stopOnDoneFade;
    if (stop)
    {
        return Result::Stop;
    }

    return Result::None;
}
