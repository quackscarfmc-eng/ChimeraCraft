// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"

namespace moonray_gui_v2 {

/// Creates a help window that displays comprehensive information about how to use the app,
/// including hotkeys that are dynamically updated from the keyboard bindings.
class HelpWindow : public Component {
public:
    HelpWindow() : Component(/*isOpen*/ false) {}
    ~HelpWindow() override {}

    // Draws the help window
    void draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset) override;

    // Get the width and height of the window
    // -1 indicates dynamic sizing (will resize with viewport)
    int getWidth() const override { return -1; }
    int getHeight() const override { return -1; }

private:
    void configureWindow() const;

    // Draw functions for different sections
    void drawCameraControls(Viewport* viewport) const;
    void drawColorManagementControls(Viewport* viewport) const;
    void drawCommandLineOptions() const;
    void drawDenoiseControls(Viewport* viewport) const;
    void drawFastProgressiveControls(Viewport* viewport) const;
    void drawOutputControls(Viewport* viewport) const;
    void drawPathVisualizerControls(Viewport* viewport) const;
    void drawStatusBarInfo() const;
    void drawViewportControls(Viewport* viewport) const;
};

} // end namespace moonray_gui_v2
