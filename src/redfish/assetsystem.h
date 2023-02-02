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

namespace rf
{
class CommandProcessor;
class DataCache;
struct AudioData;
struct Message;

class AssetSystem
{
public:
    AssetSystem(CommandProcessor* commands);
    AssetSystem(const AssetSystem&) = delete;
    AssetSystem(AssetSystem&&) = delete;
    AssetSystem& operator=(const AssetSystem&) = delete;
    AssetSystem& operator=(AssetSystem&&) = delete;
    ~AssetSystem();

    AudioHandle Load(float* interleavedSampleData, int numFrames, int channels, const char* name);
    AudioHandle Load(const char* path);
    void Unload(const AudioHandle audioHandle);

private:
    DataCache* m_dataCache = nullptr;
    CommandProcessor* m_commands = nullptr;

    AudioHandle LoadWAVFile(const char* path);
    AudioHandle LoadFLACFile(const char* path);
    const AudioData* GetAudioData(AudioHandle audioHandle) const;
    int GetAudioDataIndex(AudioHandle audioHandle) const;
    bool ProcessMessages(const Message& message);

    friend class Context;
    friend class ConvolverPlugin;
    friend class Cue;
    friend class SoundEffect;
};
}  // namespace rf