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

#include "viewbase.h"

#include <stdio.h>
#include <string.h>

rf::ViewBase::ViewBase(Context* context, const char* name, const char* nameId)
    : m_context(context)
{
    strcpy_s(m_name, name);
    strcpy_s(m_nameId, nameId);
}

void rf::ViewBase::Serialize() const {}

void rf::ViewBase::GetName(char* buffer) const
{
    if (m_isDirty)
    {
        sprintf_s(buffer, RF_MAX_NAME_SIZE, "%s*###%s", m_name, m_nameId);
    }
    else
    {
        sprintf_s(buffer, RF_MAX_NAME_SIZE, "%s###%s", m_name, m_nameId);
    }
}

void rf::ViewBase::SetDirty(bool isDirty)
{
    m_isDirty = isDirty;
}
