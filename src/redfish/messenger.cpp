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

#include "messenger.h"

#include "allocator.h"
#include "assert.h"

rf::Messenger::Messenger()
    : m_messages(RF_MAX_AUDIO_COMMANDS)
    , m_deleteMessagesToPost(RF_MAX_AUDIO_COMMANDS)
{
}

void rf::Messenger::AddMessage(const Message& message)
{
    RF_ASSERT(message.m_type != MessageType::Invalid, "Invalid message");
    m_messages.enqueue(message);
}

void rf::Messenger::AddDeleteMessage(AudioHandle audioHandle)
{
    RF_ASSERT(audioHandle, "Expected to handle to be valid");
    const bool success = m_deleteMessagesToPost.Append(audioHandle);
    RF_ASSERT(success, "Expected to be able to append");
}

bool rf::Messenger::Dequeue(Message& message)
{
    return m_messages.try_dequeue(message);
}

void rf::Messenger::FlushMessages()
{
    for (AudioHandle audioHandle : m_deleteMessagesToPost)
    {
        RF_ASSERT(audioHandle, "Expected to handle to be valid");
        Message msg;
        msg.m_type = MessageType::AssetDelete;
        msg.GetAssetDeleteData()->m_audioHandle = audioHandle;
        AddMessage(msg);
    }

    m_deleteMessagesToPost.Clear();
}