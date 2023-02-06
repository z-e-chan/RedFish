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

#include "editor.h"

#include <imgui.h>
#include <redfish/context.h>
#include <redfish/version.h>

#include <fstream>

#include "actionhandler.h"
#include "deserializeddata.h"
#include "hotkeys.h"
#include "mixerview.h"

rf::Editor::Editor(Context* context)
    : m_context(context)
{
    if (std::filesystem::exists("redfisheditor.json"))
    {
        m_deserializedData = Allocator::Allocate<DeserializedData>("DeserializedData");
        std::ifstream ifs("redfisheditor.json");
        m_deserializedData->m_json = nlohmann::json::parse(ifs);
        const auto versionJson = m_deserializedData->m_json["version"];
        m_deserializedData->m_version = Version(versionJson["major"], versionJson["minor"], versionJson["patch"]);
    }
}

rf::Editor::~Editor()
{
    for (ViewBase* view : m_views)
    {
        Allocator::Deallocate<ViewBase>(&view);
    }
    Allocator::Deallocate<DeserializedData>(&m_deserializedData);
}

void rf::Editor::Update()
{
    const rf::Version& version = rf::GetVersion();
    char buffer[64];
    sprintf_s(buffer, "RedFish v%i.%i.%i", version.m_major, version.m_minor, version.m_patch);

    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
    ImGui::Begin(buffer, nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    MenuBar();
    DrawViews();
    ImGui::End();
}

void rf::Editor::MenuBar()
{
    ActionHandler* actionHandler = m_currentView ? m_currentView->GetActionHandler() : nullptr;

    const bool undoHotKey = HotKeys::Undo();
    const bool redoHotKey = HotKeys::Redo();
    const bool canUndo = actionHandler && actionHandler->CanUndo();
    const bool canRedo = actionHandler && actionHandler->CanRedo();
    bool undo = undoHotKey && canUndo;
    bool redo = redoHotKey && canRedo;
    bool save = HotKeys::Save();

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                save = true;
            }
            if (ImGui::MenuItem("Open Mixer"))
            {
                m_views.emplace_back(Allocator::Allocate<MixerView>("MixerView", m_context, "Mixer", "MixerView", m_deserializedData));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            char undoName[128];
            if (canUndo)
            {
                sprintf_s(undoName, "Undo \"%s\"", actionHandler->GetUndoName());
            }
            else
            {
                strcpy_s(undoName, "Can't Undo");
            }

            char redoName[128];
            if (canRedo)
            {
                sprintf_s(redoName, "Redo \"%s\"", actionHandler->GetRedoName());
            }
            else
            {
                strcpy_s(redoName, "Can't Redo");
            }

            if (ImGui::MenuItem(undoName, "Ctrl+Z", false, canUndo))
            {
                undo = true;
            }
            if (ImGui::MenuItem(redoName, "Ctrl+Shift+Z", false, canRedo))
            {
                redo = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (save)
    {
        m_context->Serialize();
        m_currentView->Serialize();
        m_currentView->SetDirty(false);
    }

    if (undo)
    {
        actionHandler->UndoAction();
    }

    if (redo)
    {
        actionHandler->RedoAction();
    }
}

void rf::Editor::DrawViews()
{
    m_currentView = nullptr;

    if (ImGui::BeginTabBar("ViewTabBars"))
    {
        for (int i = 0; i < static_cast<int>(m_views.size()); ++i)
        {
            ViewBase* view = m_views[i];

            char nameBuffer[RF_MAX_NAME_SIZE];
            view->GetName(nameBuffer);
            if (ImGui::BeginTabItem(nameBuffer))
            {
                m_currentView = view;
                view->Update();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void rf::Editor::RefeshViews()
{
    for (ViewBase* view : m_views)
    {
        view->Refresh();
    }
}
