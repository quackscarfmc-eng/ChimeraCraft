// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"

namespace moonray_gui_v2 {

class SceneInspector : public Component {

public:
    SceneInspector() : Component(/*isOpen*/ false) {}
    ~SceneInspector() override {}

    void draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset) override;

    int getWidth() const override { return mWidth; }
    int getHeight() const override { return mHeight; }

private:
    // Define the window settings (size, position, style, etc)
    void configureWindow(const ImVec2& dockOffset) const;

    int mWidth {300};
    int mHeight {100};

    int mWindowPadding {10};    // padding within the window
    int mViewportPadding {20};  // padding between the window and the edges of the viewport
};

} // end namespace moonray_gui_v2
