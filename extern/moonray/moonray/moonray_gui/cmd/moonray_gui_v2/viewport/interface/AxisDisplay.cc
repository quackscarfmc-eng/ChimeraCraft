// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "AxisDisplay.h"
#include "../Viewport.h"
#include <imgui.h>

namespace moonray_gui_v2 {

void
AxisDisplay::configureWindow(const ImVec2& dockOffset)
{
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    const ImVec2 viewerSize = ImVec2(imguiViewport->Size.x - dockOffset.x, 
                                     imguiViewport->Size.y - dockOffset.y);

    // Create a transparent overlay window that covers the entire viewer area but doesn't capture input
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(viewerSize);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Calculate the origin point for the axis lines based on the viewport size and padding

    // Get the position of the window (the top left corner)
    // We can use this as the origin to draw the axis lines in the correct place
    ImVec2 windowPos = ImGui::GetWindowPos();

    // Flip y because imgui coordinates have origin at top-left, but we want to position the 
    // origin based on distance from bottom of the screen
    mOrigin = ImVec2(windowPos.x + PADDING_X, windowPos.y + viewerSize.y - PADDING_Y);
}

void
AxisDisplay::draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset)
{
    if (!mOpen) { return; }

    configureWindow(dockOffset);

    // This window will just contain the axis lines, no decorations or inputs
    if (ImGui::Begin("##AxisDisplayOverlay", nullptr, 
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoInputs |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoBringToFrontOnFocus)) {

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 start(mOrigin.x, mOrigin.y);

        auto drawAxisLine = [&](const scene_rdl2::math::Vec2f& axisDir, 
                                const scene_rdl2::math::Color& color, 
                                const char* label) {
            ImU32 lineColor = IM_COL32(color.r * 255, color.g * 255, color.b * 255, 255);

            // Offset from origin using the projected axis coordinates
            // Flip y because screen coordinates have origin at top-left
            ImVec2 end(mOrigin.x + axisDir.x * AXIS_LENGTH, mOrigin.y - axisDir.y * AXIS_LENGTH);
            drawList->AddLine(start, end, lineColor, 2.0f);

            // Calculate direction for label placement
            scene_rdl2::math::Vec2f dir(axisDir.x, axisDir.y);
            float length = scene_rdl2::math::length(dir);
            if (length > 0.001f) {
                dir = dir / length; // Normalize
            }

            // Place label at the end of the line with offset
            scene_rdl2::math::Vec2f textPos(end.x + dir.x * TEXT_OFFSET, end.y - dir.y * TEXT_OFFSET);
            ImGui::SetCursorScreenPos(ImVec2(textPos.x, textPos.y - ImGui::GetTextLineHeight() * 0.5f));
            ImGui::TextColored(ImVec4(color.r, color.g, color.b, 1.0f), label);
        };

        drawAxisLine(viewport->getAxisXDir(), scene_rdl2::math::Color(1.0f, 0.0f, 0.0f), "X");
        drawAxisLine(viewport->getAxisYDir(), scene_rdl2::math::Color(0.0f, 1.0f, 0.0f), "Y");
        drawAxisLine(viewport->getAxisZDir(), scene_rdl2::math::Color(0.54f, 0.8f, 1.0f), "Z");
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}

} // namespace moonray_gui_v2
