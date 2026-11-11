// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "SceneInspector.h"
#include "../Viewport.h"

#include <string>

namespace moonray_gui_v2 {

void
SceneInspector::configureWindow(const ImVec2& dockOffset) const
{
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    ImVec2 position(imguiViewport->Size.x - mViewportPadding - mWidth - dockOffset.x, 
                    imguiViewport->Size.y - mViewportPadding - mHeight - dockOffset.y);

    ImGui::SetNextWindowPos(position, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(mWidth, mHeight), ImGuiCond_Appearing);
}

void
SceneInspector::draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset)
{
    if (!mOpen) { return; }

    configureWindow(dockOffset);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(mWindowPadding, mWindowPadding));

    if (ImGui::Begin("Scene Inspector", &mOpen)) {
        // ------ Dropdown to select inspector mode ------
        // This list should match the options in GuiTypes::InspectorMode
        const char* items[] = { "None", "Light Contributions", "Geometry", "Geometry Part", "Material" };
        int* inspectorModePtr = viewport->getSceneInspectorModePtr();
        ImGui::Text("Mode:");
        ImGui::SameLine();
        ImGui::Combo("##Scene Inspector Mode", inspectorModePtr, items, IM_ARRAYSIZE(items));
        // ------------------------------------------------

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --------- Display inspection results -----------
        std::string text = viewport->inspect(currentPixel.x, currentPixel.y);
        ImGui::Text("%s", text.c_str());
        // ------------------------------------------------
    }
    ImGui::End();
    ImGui::PopStyleVar(1);
}

} // namespace moonray_gui_v2
