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

#include "hotkeys.h"

#include <imgui.h>

static bool ExitOut()
{
    const bool popupOpen = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    if (popupOpen)
    {
        return true;
    }

    return false;
}

static bool ControlHotKeys(int id)
{
    const ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); ++i)
    {
        if (ImGui::IsKeyPressed(id) && io.KeyCtrl && !io.KeyShift)
        {
            return true;
        }
    }
    return false;
}

static bool ControlShiftHotKeys(int id)
{
    const ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); ++i)
    {
        if (ImGui::IsKeyPressed(id) && io.KeyCtrl && io.KeyShift)
        {
            return true;
        }
    }
    return false;
}

bool rf::HotKeys::Undo()
{
    if (ExitOut())
    {
        return false;
    }

    const int zKeyId = 29;
    return ControlHotKeys(zKeyId);
}
bool rf::HotKeys::Redo()
{
    if (ExitOut())
    {
        return false;
    }

    const int zKeyId = 29;
    return ControlShiftHotKeys(zKeyId);
}

bool rf::HotKeys::Enter()
{
    const ImGuiIO& io = ImGui::GetIO();
    const int enterKeyId = 40;
    return ImGui::IsKeyPressed(enterKeyId);
}

bool rf::HotKeys::Save()
{
    const int sKeyId = 22;
    return ControlHotKeys(sKeyId);
}