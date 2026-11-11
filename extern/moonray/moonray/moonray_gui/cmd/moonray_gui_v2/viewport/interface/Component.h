// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "imgui.h"

#include <GLFW/glfw3.h>

namespace moonray_gui_v2 {

class Viewport;

class Component {
/* An abstract base class for all UI components in the ImGui interface. It 
   defines the basic interface that all components must implement, such as drawing
   the component and toggling its open/closed state.
*/
public:
    Component(bool openDefault) : mOpen(openDefault) {}
    virtual ~Component() {}
    
    // Draw the component
    virtual void draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset) = 0;

    // Toggle the window open/closed
    bool toggle() { return mOpen = !mOpen; }

    // Is the component open?
    bool isOpen() const { return mOpen; }

    // Get the width of the component
    virtual int getWidth() const = 0;

    // Get the height of the component
    virtual int getHeight() const = 0;

protected:
    bool mOpen;         // Is the window open?
};

} // end namespace moonray_gui_v2
