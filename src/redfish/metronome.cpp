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

#include "metronome.h"

#include "functions.h"
#include "messenger.h"
#include "musicdatabase.h"
#include "musictransitionrequest.h"

rf::Metronome::Metronome(const AudioSpec& spec, const MusicDatabase* musicDatabase, Messenger* messanger)
    : m_spec(spec)
    , m_messanger(messanger)
    , m_musicDatabase(musicDatabase)
{
}

void rf::Metronome::Set(float tempo, const Meter& meter)
{
    m_tempo = tempo;
    m_meter = meter;
    m_samplesPerBeat = m_beatCalculator.BeatCalc(m_tempo, m_meter, m_spec);

    Message tempoMsg;
    tempoMsg.m_type = MessageType::MusicTempo;
    tempoMsg.GetMusicTempoData()->m_tempo = m_tempo;
    m_messanger->AddMessage(tempoMsg);

    Message meterMsg;
    meterMsg.m_type = MessageType::MusicMeter;
    Message::MusicMeterData* meterData = meterMsg.GetMusicMeterData();
    meterData->m_top = m_meter.m_top;
    meterData->m_bottom = m_meter.m_bottom;
    m_messanger->AddMessage(meterMsg);
}

void rf::Metronome::Update(long long playhead, const MusicTransitionRequest& request, bool isPlaying)
{
    if (!isPlaying)
    {
        return;
    }

    const bool firstWindow = Functions::InFirstWindow(playhead, request.m_startTime, m_spec.m_bufferSize);
    if (request.m_transitionDataIndex >= 0 && firstWindow)
    {
        const MusicDatabase::TransitionData& trans = m_musicDatabase->GetTransitionData(request.m_transitionDataIndex);
        const MusicDatabase::CueData& cue = m_musicDatabase->GetCueData(trans.m_cueIndex);
        const bool tempoChange = !Functions::FloatEquality(m_tempo, cue.m_tempo);
        const bool meterChange = m_meter != cue.m_meter;
        if (tempoChange || meterChange)
        {
            Set(cue.m_tempo, cue.m_meter);
        }
    }

    if (request.m_musicLength == -1)
    {
        RF_FAIL("Music length is 0, probably due to an audio asset not being loaded");
        return;
    }

    // This can be negative when a music cue loops.
    // e.g.: a cue with a queue transition loops, the new start time ends up being very close, but just a little bigger than the playhead.
    // That means when we add in the buffer size and do the subtraction, we get a negative number.
    // In practice, this means that the cue has not played yet.
    // So by zeroing out totalMusicPlaytime in that case, we get the correct intention.
    long long totalMusicPlaytime = (playhead + m_spec.m_bufferSize) - request.m_startTime;
    if (totalMusicPlaytime < 0)
    {
        totalMusicPlaytime = 0;
    }

    const double currentBeatInTermsOfTotalBeats = floor(totalMusicPlaytime / m_samplesPerBeat);
    const double bars = floor(currentBeatInTermsOfTotalBeats / m_meter.m_top);
    const double beats = currentBeatInTermsOfTotalBeats - (bars * m_meter.m_top);

    const int lastBar = m_barCounter;
    const int lastBeat = m_beatCounter;
    m_barCounter = 1 + static_cast<int>(bars);
    m_beatCounter = 1 + static_cast<int>(beats);

    RF_ASSERT(m_barCounter > 0, "Zero bars isn't correct");
    RF_ASSERT(m_beatCounter > 0, "Zero beats isn't correct");

    if (m_barCounter != lastBar)
    {
        Message barMsg;
        barMsg.m_type = MessageType::MusicCurrentBar;
        barMsg.GetMusicCurrentBarData()->m_currentBar = m_barCounter;
        m_messanger->AddMessage(barMsg);

        Message barChangedMsg;
        barChangedMsg.m_type = MessageType::MusicBarChanged;
        Message::MusicBarChangedData* data = barChangedMsg.GetMusicBarChangedData();
        data->m_currentBar = m_barCounter;
        data->m_currentBeat = m_beatCounter;
        m_messanger->AddMessage(barChangedMsg);
    }
    if (m_beatCounter != lastBeat)
    {
        Message beatMsg;
        beatMsg.m_type = MessageType::MusicCurrentBeat;
        beatMsg.GetMusicCurrentBeatData()->m_currentBeat = m_beatCounter;
        m_messanger->AddMessage(beatMsg);

        Message beatChangedMsg;
        beatChangedMsg.m_type = MessageType::MusicBeatChanged;
        Message::MusicBeatChangedData* data = beatChangedMsg.GetMusicBeatChangedData();
        data->m_currentBar = m_barCounter;
        data->m_currentBeat = m_beatCounter;
        m_messanger->AddMessage(beatChangedMsg);
    };
}

void rf::Metronome::Reset()
{
    m_beatCalculator = BeatCalculator();
    m_meter = Meter();
    m_tempo = -1.0f;
    m_samplesPerBeat = 0.0;
    m_barCounter = 0;
    m_beatCounter = 0;

    Message barMsg;
    barMsg.m_type = MessageType::MusicCurrentBar;
    barMsg.GetMusicCurrentBarData()->m_currentBar = 0;
    m_messanger->AddMessage(barMsg);

    Message beatMsg;
    beatMsg.m_type = MessageType::MusicCurrentBeat;
    beatMsg.GetMusicCurrentBeatData()->m_currentBeat = 0;
    m_messanger->AddMessage(beatMsg);
}

int rf::Metronome::GetSyncSamples(const AudioSpec& spec, const Sync& sync, float tempo, const Meter& meter)
{
    if (sync.m_mode == Sync::Mode::Time)
    {
        return static_cast<int>(round(spec.m_sampleRate * sync.m_time));
    }

    BeatCalculator beatCalc;
    beatCalc.BeatCalc(tempo, meter, spec);
    return beatCalc.BeatSwitch(sync);
}

double rf::Metronome::PreciseBeatSwitch(const Sync& sync) const
{
    return m_beatCalculator.PreciseBeatSwitch(sync);
}

float rf::Metronome::GetTempo() const
{
    return m_tempo;
}

rf::Meter rf::Metronome::GetMeter() const
{
    return m_meter;
}

int rf::Metronome::GetBarCounter() const
{
    return m_barCounter;
}

int rf::Metronome::GetBeatCounter() const
{
    return m_beatCounter;
}
