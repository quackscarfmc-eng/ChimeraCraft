// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"

namespace moonray_gui_v2 {

class Viewport;

class AxisDisplay : public Component {
public:
    AxisDisplay() : Component(/*isOpen*/ true) {}
    ~AxisDisplay() override {}

    // Draw the axis display (X, Y, Z) in the bottom left corner of the viewport
    void draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset) override;

    // Get width (not applicable for overlay component)
    int getWidth() const override { return 0; }

    // Get height (not applicable for overlay component)
    int getHeight() const override { return 0; }

private:
    void configureWindow(const ImVec2& dockOffset);

    ImVec2 mOrigin {ImVec2(0, 0)};              // The origin point for the axis lines, in viewport coordinates
    static constexpr float AXIS_LENGTH = 25.0f; // Length of the axis lines in pixels
    static constexpr float TEXT_OFFSET = 9.f;   // Offset for the axis labels from the end of the axis lines
    static constexpr float PADDING_X = 50.0f;   // Padding from the left edge of the viewport
    static constexpr float PADDING_Y = 25.0f;   // Padding from the bottom edge of the viewport
};

} // namespace moonray_gui_v2
