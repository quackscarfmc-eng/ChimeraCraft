// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "DrawUtil.h"

namespace moonray_gui_v2 {

// Add a dashed line to the imgui draw list, given the start/endpoint
void addDashedLine(const ImVec2& start, const ImVec2& end, const float thickness, const ImU32 color)
{
    // Calculate the direction and length of the line
    ImVec2 direction(end.x - start.x, end.y - start.y);
    const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    direction.x /= length;
    direction.y /= length;

    // Calculate the number of segments for the dashed line
    static constexpr int segmentLength = 15; // in pixels
    int numSegments = static_cast<int>(length / segmentLength);
    if (numSegments < 1) numSegments = 1;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (int i = 0; i < numSegments; ++i) {
        const float t0 = static_cast<float>(i) / numSegments;
        const float t1 = static_cast<float>(i + 0.5f) / numSegments;
        const ImVec2 p0(start.x + direction.x * length * t0, start.y + direction.y * length * t0);
        const ImVec2 p1(start.x + direction.x * length * t1, start.y + direction.y * length * t1);
        drawList->AddLine(p0, p1, color, thickness);
    }
}

void drawInputFloat(const std::string& label, float value, const std::function<void(float)>& callback)
{
    const std::string text = label + ": ";
    const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    // Calculate the width needed for the input box
    // Get the space available, and subtract the text size and 5px padding
    const int remainingWidth = ImGui::GetContentRegionAvail().x - textSize.x - 5;

    // ------- Draw the label --------
    ImGui::Text("%s", text.c_str());
    ImGui::SameLine();

    // ------- Draw the input box --------
    ImGui::SetNextItemWidth(remainingWidth);
    const std::string id = "##" + label;
    ImGui::InputFloat(id.c_str(), &value, 0.0f, 0.0f);
    
    // Only register the value when the input loses focus or user presses Enter
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        callback(value);
    }
}
// Draw an integer input box with a label, and a callback for when the value changes
void drawInputInt(const std::string& label, int value, const int min, const int max, 
                  const std::function<void(int)>& callback)
{
    const std::string text = label + ": ";
    const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    // Calculate the width needed for the input box
    // Get the space available, and subtract the text size and 5px padding
    const int remainingWidth = ImGui::GetContentRegionAvail().x - textSize.x - 5;

    // ------- Draw the label --------
    ImGui::Text("%s", text.c_str());
    ImGui::SameLine();

    // ------- Draw the input box --------
    ImGui::SetNextItemWidth(remainingWidth);
    const std::string id = "##" + label;
    if (ImGui::InputInt(id.c_str(), &value, min, max)) {
        callback(value);
    }
}

// Draw a slider with a label to the left
void drawSliderFloat(const std::string& label, float value, const float min, const float max, 
                  const std::function<void(float)>& callback)
{
    const std::string text = label + ": ";
    const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    // Calculate the width needed for the input box
    // Get the space available, and subtract the text size and 5px padding
    const int remainingWidth = ImGui::GetContentRegionAvail().x - textSize.x - 5;

    // ------- Draw the label --------
    ImGui::Text("%s", text.c_str());
    ImGui::SameLine();

    // ------- Draw the input box --------
    ImGui::SetNextItemWidth(remainingWidth);
    const std::string id = "##" + label;
    if (ImGui::SliderFloat(id.c_str(), &value, min, max)) {
        callback(value);
    }
}

// Draw a color edit with a label, and a callback for when the value changes
void drawColorEdit(const std::string& label, const scene_rdl2::math::Color& color, 
                   const std::function<void(scene_rdl2::math::Color)>& callback)
{
    float colorArray[3] = {color.r, color.g, color.b};
    if (ImGui::ColorEdit3(label.c_str(), colorArray, ImGuiColorEditFlags_NoInputs)) {
        callback(scene_rdl2::math::Color(colorArray[0], colorArray[1], colorArray[2]));
    }
    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
}

} // end namespace moonray_gui_v2