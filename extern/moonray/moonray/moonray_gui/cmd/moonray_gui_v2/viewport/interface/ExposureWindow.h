// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"

namespace moonray_gui_v2 {

class ExposureWindow : public Component {
public:
    ExposureWindow() : Component(/*isOpen*/ false) {}
    
    // Draw the window
    void draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset) override;

    int getWidth() const override { return mWidth; }
    int getHeight() const override { return mHeight; }

private:
    // Define the window settings (size, position, style, etc)
    void configureWindow() const;

    int mWidth {105};           // Width of the window
    int mHeight {77};           // Height of the window

    float mMinExposure {-10.f};
    float mMaxExposure {10.f};
    float mExposureDefault {0.f};
};

} // end namespace moonray_gui_v2
