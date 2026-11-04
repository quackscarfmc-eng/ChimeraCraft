// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "imgui.h"

#include <scene_rdl2/common/math/Color.h>

#include <functional> // for std::function
#include <string>

namespace moonray_gui_v2 {

/// Add a dashed line to the imgui draw list, given the start/endpoint
void addDashedLine(const ImVec2& start, const ImVec2& end, float thickness, const ImU32 color);

// Draw a float input box with a label, and a callback for when the value changes
void drawInputFloat(const std::string& label, float value, const std::function<void(float)>& callback);

// Draw an integer input box with a label, and a callback for when the value changes
void drawInputInt(const std::string& label, int value, const int min, const int max, 
                  const std::function<void(int)>& callback);

// Draw a float input box with a label, and a callback for when the value changes
void drawSliderFloat(const std::string& label, float value, const float min, const float max, 
                  const std::function<void(float)>& callback);

// Draw a color edit with a label, and a callback for when the value changes
void drawColorEdit(const std::string& label, const scene_rdl2::math::Color& color, 
                   const std::function<void(scene_rdl2::math::Color)>& callback);

} // end namespace moonray_gui_v2