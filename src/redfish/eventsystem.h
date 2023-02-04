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

namespace rf
{
class CommandProcessor;
struct Message;

class EventSystem
{
public:
    EventSystem(CommandProcessor* commands);
    EventSystem(const EventSystem&) = delete;
    EventSystem(EventSystem&&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    EventSystem& operator=(EventSystem&&) = delete;
    ~EventSystem() = default;

    void SetUserData(void* userData);
    void RegisterOnBar(void (*onBar)(int bar, int beat, void* userData));
    void RegisterOnBeat(void (*onBeat)(int bar, int beat, void* userData));
    void RegisterOnMusicFinished(void (*onMusicFinished)(void* userData));

private:
    CommandProcessor* m_commands = nullptr;
    void* m_userData = nullptr;
    void (*m_onBar)(int bar, int beat, void* userData) = nullptr;
    void (*m_onBeat)(int bar, int beat, void* userData) = nullptr;
    void (*m_onMusicFinished)(void* userData) = nullptr;

    void ProcessMessages(const Message& message);

    friend class Context;
};
}  // namespace rf