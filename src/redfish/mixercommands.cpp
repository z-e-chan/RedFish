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

#include "mixercommands.h"

#include "audiotimeline.h"
#include "functions.h"

rf::AudioCommandCallback rf::CreateMixGroupCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const CreateMixGroupCommand& cmd = *static_cast<CreateMixGroupCommand*>(command);
    timeline->m_summingMixer.CreateMixGroup(cmd.m_mixGroupState);
};

rf::AudioCommandCallback rf::SetMixGroupAmplitudeCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SetMixGroupAmplitudeCommand& cmd = *static_cast<SetMixGroupAmplitudeCommand*>(command);
    SummingMixer::MixGroupInternal* mixGroup = timeline->m_summingMixer.MixGroupLookUp(cmd.m_mixGroupHandle);
    mixGroup->m_state.m_volumeDb = Functions::AmplitudeToDecibel(cmd.m_amplitude);
    mixGroup->UpdateVolume(cmd.m_amplitude, 0.01f);
};

rf::AudioCommandCallback rf::SetMixGroupOutputCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SetMixGroupOutputCommand& cmd = *static_cast<SetMixGroupOutputCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    SummingMixer::MixGroupInternal* mixGroup = mixer->MixGroupLookUp(cmd.m_mixGroupHandle);
    mixGroup->m_state.m_priority = cmd.m_priority;
    mixGroup->m_state.m_outputMixGroupHandle = cmd.m_outputMixGroupHandle;
    mixer->Sort();
};

rf::AudioCommandCallback rf::CreateSendCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const CreateSendCommand& cmd = *static_cast<CreateSendCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    SummingMixer::MixGroupInternal* mixGroup = mixer->MixGroupLookUp(cmd.m_mixGroupHandle);
    mixGroup->m_state.m_priority = cmd.m_priority;
    mixGroup->m_state.m_sendSlots[cmd.m_mixGroupSlot] = cmd.m_sendIndex;
    mixer->m_sends[cmd.m_sendIndex].m_sendToMixGroupHandle = cmd.m_sendToMixGroupHandle;
    mixer->Sort();
};

rf::AudioCommandCallback rf::DestroySendCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const DestroySendCommand& cmd = *static_cast<DestroySendCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    SummingMixer::MixGroupInternal* mixGroup = mixer->MixGroupLookUp(cmd.m_mixGroupHandle);
    mixGroup->m_state.m_priority = cmd.m_priority;
    mixGroup->m_state.m_sendSlots[cmd.m_mixGroupSlot] = -1;
    mixer->m_sends[cmd.m_sendIndex] = SummingMixer::SendInternal();
    mixer->Sort();
};

rf::AudioCommandCallback rf::SetSendAmplitudeCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SetSendAmplitudeCommand& cmd = *static_cast<SetSendAmplitudeCommand*>(command);
    SummingMixer* mixer = &timeline->m_summingMixer;
    mixer->m_sends[cmd.m_sendIndex].m_amplitude = cmd.m_amplitude;
};

rf::AudioCommandCallback rf::FadeMixGroupsCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const FadeMixGroupsCommand& cmd = *static_cast<FadeMixGroupsCommand*>(command);
    const long long playhead = timeline->GetPlayhead();

    const MusicManager& musicManager = timeline->m_musicManager;
    long long startTime = musicManager.CalculateStartTime(cmd.m_sync, playhead);
    const int duration = musicManager.GetSyncSamples(cmd.m_duration);

    if (cmd.m_stingerIndex >= 0)
    {
        const MusicDatabase* database = musicManager.GetMusicDatabase();
        const MusicDatabase::StingerData& stingerData = database->GetStingerData(cmd.m_stingerIndex);
        const int stingerDuration = musicManager.GetSyncSamples(stingerData.m_sync);

        // Not enough time to play the stinger.
        if (startTime - stingerDuration < playhead)
        {
            // Extend the transition time further in time to accommodate the stinger.
            const int extension = musicManager.GetSyncSamples(cmd.m_sync);
            const int diff = cmd.m_sync.m_sync == Sync::Value::Cut ? 1 : 0;

            const long long d1 = startTime - stingerDuration;
            const long long d2 = playhead - d1;
            const long long div = (d2 / extension) + 1;
            startTime = ((div * extension) + startTime) - diff;
        }

        timeline->m_voiceSet.CreateVoice(timeline->m_audioDataReferences, stingerData, startTime - stingerDuration, database);
    }

    SummingMixer& mixer = timeline->m_summingMixer;

    for (int i = 0; i < cmd.m_numMixGroups; ++i)
    {
        SummingMixer::MixGroupInternal* mixGroup = mixer.MixGroupLookUp(cmd.m_mixGroupIndices[i]);
        mixGroup->FadeVolume(cmd.m_amplitude, playhead, startTime, duration);
    }
};