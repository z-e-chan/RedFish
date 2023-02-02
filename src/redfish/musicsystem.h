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

#pragma once
#include "identifiers.h"
#include "meter.h"
#include "transitioncondition.h"

namespace rf
{
class AssetSystem;
class CommandProcessor;
class Cue;
class Stinger;
class Transition;
struct CueParameters;
struct Message;
struct StingerParameters;
struct Sync;
struct TransitionParameters;

class MusicSystem
{
public:
    MusicSystem(CommandProcessor* commands, AssetSystem* assetSystem);
    MusicSystem(const MusicSystem&) = delete;
    MusicSystem(MusicSystem&&) = delete;
    MusicSystem& operator=(const MusicSystem&) = delete;
    MusicSystem& operator=(MusicSystem&&) = delete;
    ~MusicSystem();

    struct UserData
    {
        uint8_t m_data[TransitionCondition::k_dataSize] = {};
    };

    Cue* CreateCue(const CueParameters& parameters);
    void DestroyCue(Cue** cue);
    Transition* CreateTransition(const TransitionParameters& parameters);
    void DestroyTransition(Transition** transition);
    Stinger* CreateStinger(const StingerParameters& parameters);
    void DestroyStinger(Stinger** stinger);
    void Play(const Transition* transition) const;
    void Play(UserData userData) const;
    void Play(const Stinger* stinger) const;
    void Stop() const;
    void FadeOutAndStop(const Sync& sync, const Sync& duration) const;
    CueHandle GetCurrentCueHandle() const;
    const char* GetCurrentCueName() const;
    const Meter& GetCurrentMeter() const;
    int GetCurrentBar() const;
    int GetCurrentBeat() const;
    float GetCurrentTempo() const;

private:
    CommandProcessor* m_commands = nullptr;
    AssetSystem* m_assetSystem = nullptr;
    Cue* m_cues = nullptr;
    Transition* m_transitions = nullptr;
    Stinger* m_stingers = nullptr;
    CueHandle m_currentCueHandle;
    Meter m_meter;
    int m_bar = 0;
    int m_beat = 0;
    float m_tempo = 0.0f;

    bool ProcessMessages(const Message& message);

    friend class Context;
};
}  // namespace rf