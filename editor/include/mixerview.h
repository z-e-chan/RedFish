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
#include <vector>

#include "mixgroupeditorstate.h"
#include "viewbase.h"

namespace rf
{
class ActionHandler;
class MixerSystem;
class MixGroup;
struct DeserializedData;

class MixerView final : public ViewBase
{
public:
    MixerView(Context* context, const char* name, const char* nameId, const DeserializedData* data);
    ~MixerView();

    void Refresh() override;
    void Update() override;
    ActionHandler* GetActionHandler() override;
    void Serialize() const override;
    MixGroupEditorState& GetEditorState(int index);
    void SortEditorState();

private:
    std::vector<MixGroupEditorState> m_editorState;
    ActionHandler* m_actions = nullptr;
    MixerSystem* m_mixerSystem = nullptr;

    void SetDirtyToTrue();
    int GetEditorStateIndex(const MixGroup* mixGroup);
    bool MixGroupComboBox(const char* label, int* inOutIndex, int ignoreIndex);
    void VisibiltiyPane();
    void MixGroups();
    void MixGroupStrip(MixGroupEditorState& editorState, int index);
    void Plugins(MixGroupEditorState& editorState, int index);
    void Sends(MixGroupEditorState& editorState, int index);
    void Fader(MixGroupEditorState& editorState);
    void Output(MixGroupEditorState& editorState, int index);
    void Name(MixGroupEditorState& editorState, int index);
};
}  // namespace rf