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

#include "playcommands.h"

#include "audiotimeline.h"

rf::AudioCommandCallback rf::PlayCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const PlayCommand& cmd = *static_cast<PlayCommand*>(command);
    const AudioData* data = timeline->GetAudioData(cmd.m_audioDataIndex);
    timeline->m_voiceSet.CreateVoice(data, cmd, timeline->GetPlayhead() + 1000);
};

rf::AudioCommandCallback rf::StopSoundEffectCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const StopSoundEffectCommand& cmd = *static_cast<StopSoundEffectCommand*>(command);
    const long long playhead = timeline->GetPlayhead();
    timeline->m_voiceSet.StopBySoundEffectHandle(cmd.m_soundEffectHandle, playhead, playhead);
};

rf::AudioCommandCallback rf::SoundEffectAmplitudeCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SoundEffectAmplitudeCommand& cmd = *static_cast<SoundEffectAmplitudeCommand*>(command);
    timeline->m_voiceSet.SetAmplitudeBySoundEffectHandle(cmd.m_soundEffectHandle, cmd.m_amplitude);
};

rf::AudioCommandCallback rf::SoundEffectPitchCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SoundEffectPitchCommand& cmd = *static_cast<SoundEffectPitchCommand*>(command);
    timeline->m_voiceSet.SetPitchBySoundEffectHandle(cmd.m_soundEffectHandle, cmd.m_pitch);
};

rf::AudioCommandCallback rf::SoundEffectPositioningParamtersCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SoundEffectPositioningParamtersCommand& cmd = *static_cast<SoundEffectPositioningParamtersCommand*>(command);
    timeline->m_voiceSet.SetPositionBySoundEffectHandle(cmd.m_soundEffectHandle, cmd.m_positioningParameters, true);
};

rf::AudioCommandCallback rf::SoundEffectFadeCommondCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const SoundEffectFadeCommondCommand& cmd = *static_cast<SoundEffectFadeCommondCommand*>(command);
    const long long playhead = timeline->GetPlayhead();

    const MusicManager& musicManager = timeline->m_musicManager;
    const int sampleDuration = cmd.m_sync.m_mode == Sync::Mode::Musical
                                   ? musicManager.GetSyncSamples(cmd.m_duration)
                                   : static_cast<int>(round(cmd.m_duration.m_time * timeline->m_spec.m_sampleRate));
    const long long startTime = musicManager.CalculateStartTime(cmd.m_sync, playhead);

    timeline->m_voiceSet.Fade(cmd.m_soundEffectHandle, startTime, cmd.m_amplitude, sampleDuration, playhead, cmd.m_stopOnDone);
};