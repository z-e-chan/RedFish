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

#include "loadcommands.h"

#include "audiotimeline.h"

rf::AudioCommandCallback rf::LoadAudioDataCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const LoadAudioDataCommand& cmd = *static_cast<LoadAudioDataCommand*>(command);
    timeline->m_audioDataReferences[cmd.m_index] = cmd.m_audioData;
};

rf::AudioCommandCallback rf::UnloadAudioDataCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const UnloadAudioDataCommand& cmd = *static_cast<UnloadAudioDataCommand*>(command);
    const long long playhead = timeline->GetPlayhead();
    timeline->m_voiceSet.Unload(cmd.m_audioHandle, playhead);
    timeline->m_musicManager.Unload(cmd.m_audioHandle, playhead);
    timeline->m_messenger.AddDeleteMessage(cmd.m_audioHandle);
};

rf::AudioCommandCallback rf::ClearAudioDataReferenceCommand::s_callback = [](AudioTimeline* timeline, void* command) {
    const ClearAudioDataReferenceCommand& cmd = *static_cast<ClearAudioDataReferenceCommand*>(command);
    RF_ASSERT(cmd.m_index >= 0 && cmd.m_index < RF_MAX_AUDIO_DATA, "Index out of bounds");
    timeline->m_audioDataReferences[cmd.m_index] = nullptr;
};

rf::AudioCommandCallback rf::ShutdownCommand::s_callback = [](AudioTimeline* timeline, void*) { timeline->Shutdown(); };