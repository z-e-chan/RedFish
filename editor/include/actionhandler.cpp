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

#include "actionhandler.h"

#include "action.h"

rf::ActionHandler::ActionHandler(std::function<void()> onDoAction)
    : m_onDoAction(onDoAction)
{
    m_actions.reserve(2048);
}

rf::ActionHandler::~ActionHandler()
{
    DeleteActions();
}

void rf::ActionHandler::DoAction(Action* action)
{
    m_actions.resize(m_currentActionIndex + 1);
    m_actions[m_currentActionIndex] = action;
    ++m_currentActionIndex;
    action->Do();

    if (m_onDoAction)
    {
        m_onDoAction();
    }
}

bool rf::ActionHandler::CanUndo()
{
    return m_currentActionIndex > 0;
}

bool rf::ActionHandler::CanRedo()
{
    return m_currentActionIndex < m_actions.size();
}

const char* rf::ActionHandler::GetUndoName()
{
    if (CanUndo())
    {
        const Action* action = m_actions[m_currentActionIndex - 1];
        return action->GetName();
    }

    return "";
}

const char* rf::ActionHandler::GetRedoName()
{
    if (CanRedo())
    {
        const Action* action = m_actions[m_currentActionIndex];
        return action->GetName();
    }

    return "";
}

void rf::ActionHandler::UndoAction()
{
    if (CanUndo())
    {
        Action* action = m_actions[--m_currentActionIndex];
        action->Undo();
    }
}

void rf::ActionHandler::RedoAction()
{
    if (CanRedo())
    {
        Action* action = m_actions[m_currentActionIndex++];
        action->Do();
    }
}

void rf::ActionHandler::DeleteActions()
{
    for (Action* action : m_actions)
    {
        delete action;
    }

    m_actions.clear();
}