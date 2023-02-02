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

#include "stinger.h"

#include <string.h>

#include "cue.h"

rf::Stinger::Stinger(const StingerParameters& stingerParameters, int index)
    : m_stingerHandle(CreateStingerHandle())
    , m_cueHandle(stingerParameters.m_cue ? stingerParameters.m_cue->GetCueHandle() : CueHandle())
    , m_sync(stingerParameters.m_sync)
    , m_gainDb(stingerParameters.m_gainDb)
    , m_index(index)
{
    if (stingerParameters.m_name)
    {
        strcpy_s(m_name, stingerParameters.m_name);
    }
}

rf::Stinger::operator bool() const
{
    return static_cast<bool>(m_cueHandle);
}

rf::StingerHandle rf::Stinger::GetStingerHandle() const
{
    return m_stingerHandle;
}

rf::CueHandle rf::Stinger::GetCueHandle() const
{
    return m_cueHandle;
}

const rf::Sync& rf::Stinger::GetSync() const
{
    return m_sync;
}

float rf::Stinger::GetGainDb() const
{
    return m_gainDb;
}

const char* rf::Stinger::GetName() const
{
    return m_name;
}

int rf::Stinger::GetIndex() const
{
    return m_index;
}