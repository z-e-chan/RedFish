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
#include "assert.h"
#include "defines.h"
#include "identifiers.h"

namespace rf
{
#define RF_MESSAGE(data, type)                            \
    data* Get##data()                                     \
    {                                                     \
        RF_ASSERT(type == m_type, "Wrong message type."); \
        return reinterpret_cast<data*>(m_data);           \
    }                                                     \
    const data* Get##data() const                         \
    {                                                     \
        RF_ASSERT(type == m_type, "Wrong message type."); \
        return reinterpret_cast<const data*>(m_data);     \
    }                                                     \
    static_assert(sizeof(data) <= k_maxDataSize, "Exceeded max data");

enum class MessageType
{
    Invalid,

    AssetDelete,
    ContextNumVoices,
    ContextShutdownComplete,
    ContextVoiceStart,
    ContextVoiceStop,
    MixGroupFadeComplete,
    MixGroupPeakAmplitude,
    MusicBarChanged,
    MusicBeatChanged,
    MusicCurrentBar,
    MusicCurrentBeat,
    MusicDestroyCue,
    MusicDestroyStinger,
    MusicDestroyTransition,
    MusicFinished,
    MusicMeter,
    MusicTempo,
    MusicTransitioned,
};

struct Message
{
    MessageType m_type = MessageType::Invalid;

    struct AssetDeleteData
    {
        AudioHandle m_audioHandle;
    };

    struct ContextNumVoicesData
    {
        int m_numVoices;
    };

    struct ContextVoiceStartData
    {
        AudioHandle m_audioHandle;
    };

    struct ContextVoiceStopData
    {
        AudioHandle m_audioHandle;
    };

    struct MixGroupFadeCompleteData
    {
        MixGroupHandle m_mixGroupHandle;
        float m_amplitude;
    };

    struct MixGroupPeakAmplitudeData
    {
        int m_mixGroupIndex = -1;
        float m_amplitude = 0.0f;
    };

    struct MusicBarChangedData
    {
        int m_currentBar;
        int m_currentBeat;
    };

    struct MusicBeatChangedData
    {
        int m_currentBar;
        int m_currentBeat;
    };

    struct MusicCurrentBarData
    {
        int m_currentBar;
    };

    struct MusicCurrentBeatData
    {
        int m_currentBeat;
    };

    struct MusicDestroyCueData
    {
        int m_index;
    };

    struct MusicDestroyStingerData
    {
        int m_index;
    };

    struct MusicDestroyTransitionData
    {
        int m_index;
    };

    struct MusicMeterData
    {
        int m_top;
        int m_bottom;
    };

    struct MusicTempoData
    {
        float m_tempo;
    };

    struct MusicTransitionedData
    {
        CueHandle m_toCueHandle;
        CueHandle m_fromCueHandle;
    };

    static constexpr int k_maxDataSize = 8;
    uint8_t m_data[k_maxDataSize];

    RF_MESSAGE(AssetDeleteData, MessageType::AssetDelete);
    RF_MESSAGE(ContextNumVoicesData, MessageType::ContextNumVoices);
    RF_MESSAGE(ContextVoiceStartData, MessageType::ContextVoiceStart);
    RF_MESSAGE(ContextVoiceStopData, MessageType::ContextVoiceStop);
    RF_MESSAGE(MixGroupFadeCompleteData, MessageType::MixGroupFadeComplete);
    RF_MESSAGE(MixGroupPeakAmplitudeData, MessageType::MixGroupPeakAmplitude);
    RF_MESSAGE(MusicBarChangedData, MessageType::MusicBarChanged);
    RF_MESSAGE(MusicBeatChangedData, MessageType::MusicBeatChanged);
    RF_MESSAGE(MusicCurrentBarData, MessageType::MusicCurrentBar);
    RF_MESSAGE(MusicCurrentBeatData, MessageType::MusicCurrentBeat);
    RF_MESSAGE(MusicDestroyCueData, MessageType::MusicDestroyCue);
    RF_MESSAGE(MusicDestroyStingerData, MessageType::MusicDestroyStinger);
    RF_MESSAGE(MusicDestroyTransitionData, MessageType::MusicDestroyTransition);
    RF_MESSAGE(MusicMeterData, MessageType::MusicMeter);
    RF_MESSAGE(MusicTempoData, MessageType::MusicTempo);
    RF_MESSAGE(MusicTransitionedData, MessageType::MusicTransitioned);
};

#undef RF_MESSAGE
}  // namespace rf