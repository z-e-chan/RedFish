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

#include "musiccommands.h"

#include "audiotimeline.h"

rf::AudioCommandCallback rf::CreateCueCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const CreateCueCommand& cmd = *static_cast<CreateCueCommand*>(command);
    MusicDatabase* database = timeline->m_musicManager.GetMusicDatabase();
    database->CreateCue(cmd.m_cue, cmd.m_index);
};

rf::AudioCommandCallback rf::DestroyCueCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const DestroyCueCommand& cmd = *static_cast<DestroyCueCommand*>(command);
    MusicManager& musicManager = timeline->m_musicManager;
    musicManager.DestroyCue(cmd.m_index, timeline->GetPlayhead());
    MusicDatabase* database = musicManager.GetMusicDatabase();
};

rf::AudioCommandCallback rf::CreateTransitionCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const CreateTransitionCommand& cmd = *static_cast<CreateTransitionCommand*>(command);
    MusicDatabase* database = timeline->m_musicManager.GetMusicDatabase();
    database->CreateTransition(cmd.m_transition, cmd.m_index);
};

rf::AudioCommandCallback rf::DestroyTransitionCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const DestroyTransitionCommand& cmd = *static_cast<DestroyTransitionCommand*>(command);
    MusicManager& musicManager = timeline->m_musicManager;
    musicManager.DestroyTransition(cmd.m_index, timeline->GetPlayhead());
    MusicDatabase* database = musicManager.GetMusicDatabase();
};

rf::AudioCommandCallback rf::CreateStingerCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const CreateStingerCommand& cmd = *static_cast<CreateStingerCommand*>(command);
    MusicDatabase* database = timeline->m_musicManager.GetMusicDatabase();
    database->CreateStinger(cmd.m_stinger, cmd.m_index);
};

rf::AudioCommandCallback rf::DestroyStingerCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const DestroyTransitionCommand& cmd = *static_cast<DestroyTransitionCommand*>(command);
    MusicManager& musicManager = timeline->m_musicManager;
    MusicDatabase* database = musicManager.GetMusicDatabase();
    const long long& playhead = timeline->GetPlayhead();
    timeline->m_voiceSet.StopByStingerHandle(database->GetStingerData(cmd.m_index).m_stingerHandle, playhead, playhead);
    musicManager.DestroyStinger(cmd.m_index, playhead);
};

rf::AudioCommandCallback rf::PlayTransitionCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const PlayTransitionCommand& cmd = *static_cast<PlayTransitionCommand*>(command);
    timeline->m_musicManager.AddTransition(cmd.m_index);
};

rf::AudioCommandCallback rf::PickTransitionCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const PickTransitionCommand& cmd = *static_cast<PickTransitionCommand*>(command);
    MusicManager& musicManager = timeline->m_musicManager;
    musicManager.PickAndAddTransition(cmd.m_userData);
};

rf::AudioCommandCallback rf::StopMusicCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const StopMusicCommand& cmd = *static_cast<StopMusicCommand*>(command);
    const long long playhead = timeline->GetPlayhead();
    MusicManager& musicManager = timeline->m_musicManager;
    const long long stopTime = musicManager.CalculateStartTime(cmd.m_sync, playhead);
    musicManager.Stop(stopTime, playhead);
    timeline->m_voiceSet.StopIfIsStinger(stopTime, playhead);
};

rf::AudioCommandCallback rf::FadeOutAndStopMusicCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const FadeOutAndStopMusicCommand& cmd = *static_cast<FadeOutAndStopMusicCommand*>(command);
    MusicManager& musicManager = timeline->m_musicManager;
    const long long playhead = timeline->GetPlayhead();
    const int sampleDuration = cmd.m_sync.m_mode == Sync::Mode::Musical
                                   ? musicManager.GetSyncSamples(cmd.m_duration)
                                   : static_cast<int>(round(cmd.m_duration.m_time * timeline->m_spec.m_sampleRate));
    const long long startTime = musicManager.CalculateStartTime(cmd.m_sync, playhead);
    musicManager.Fade(startTime, 0.0f, sampleDuration, playhead, true);
};

rf::AudioCommandCallback rf::PlayStingerCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const PlayStingerCommand& cmd = *static_cast<PlayStingerCommand*>(command);
    MusicDatabase* database = timeline->m_musicManager.GetMusicDatabase();
    const MusicDatabase::StingerData& stingerData = database->GetStingerData(cmd.m_index);
    MusicManager& musicManager = timeline->m_musicManager;
    const long long playhead = timeline->GetPlayhead();
    const long long startTime = musicManager.CalculateStartTime(stingerData.m_sync, timeline->GetPlayhead());
    timeline->m_voiceSet.CreateVoice(timeline->m_audioDataReferences, stingerData, startTime, database);
};