// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"
#include "Dock.h"
#include "../../GuiTypes.h"
#include "ImageDisplay.h"

#include "imgui.h"

#include <GLFW/glfw3.h>
#include <memory>   // for std::unique_ptr

namespace moonray_gui_v2 {

class AxisDisplay;
class ExposureWindow;
class GammaWindow;
class HelpWindow;
class ImageDisplay;
class KeyBindingsWindow;
class PathVisualizerWindow;
class PixelInspector;
class SceneInspector;
class SnapshotWindow;
class StatusBar;
class Viewport;

class Interface
{
    /* This class manages the ImGui user interface for the application. Any key/mouse interactions in the 
       viewport (i.e. the area that contains the rendered image) are managed by GLFW in the Viewport class. 
       However, any interactions with menus or interactive windows are managed by this class. */

public:
    Interface(Viewport* viewport);
    ~Interface();

    // Draw the UI
    void draw();

    // Sleep for the designated number of ms
    void sleep(const int milliseconds = 10);

    // For the most part, Viewport will handle the key/mouse events (unless a 
    // imgui window is in focus), but hotkeys for opening/closing windows will be handled here
    bool handleKeyPressEvent(const Action action);

    // Resize the image display
    void resizeImage(const int width, const int height);

    // Toggle open/closed windows
    void toggleAxisDisplay();
    void toggleExposureWindow();
    void toggleGammaWindow();
    void toggleHelpWindow();
    void toggleKeyBindings();
    void togglePathVisualizerWindow();
    void togglePixelInspector();
    void toggleSceneInspector();
    void toggleSnapshotWindow();
    void toggleStatusBar();

    // Get the width/height of the docked components
    // We need this to calculate the new viewport size when 
    // docked components (i.e. ones that affect viewport size)
    // are opened or closed
    int getRightDockWidth() const { return mRightDock.getWidth(); }
    int getBottomDockHeight() const { return mBottomDock.getHeight(); }

    // Get the pixel currently underneath the mouse
    ImVec2 getCurrentPixel() const;

    // Zoom in/out or pan the image display
    void zoom(const int xoffset, const int yoffset);
    void pan(const int xoffset, const int yoffset);

private:

    // Resize the viewport, if necessary (usually because
    // of stationary ui elements opening or closing)
    void resizeViewport();    

    // Render the UI
    void render();

    // Create a new frame
    void newFrame();

    Viewport* mViewport {nullptr};                               // ptr to the viewport manager
    std::unique_ptr<AxisDisplay> mAxisDisplay;                   // ptr to axis display component
    std::unique_ptr<ExposureWindow> mExposureWindow;             // ptr to exposure adjustment window
    std::unique_ptr<GammaWindow> mGammaWindow;                   // ptr to gamma adjustment window
    std::unique_ptr<HelpWindow> mHelpWindow;                     // ptr to help window
    std::unique_ptr<KeyBindingsWindow> mKeyBindingsWindow;       // ptr to keybindings window
    std::unique_ptr<ImageDisplay> mImageDisplay;                 // ptr to gui component displaying the rendered image
    std::unique_ptr<PathVisualizerWindow> mPathVisualizerWindow; // ptr to path visualizer window
    std::unique_ptr<PixelInspector> mPixelInspector;             // ptr to pixel inspector window
    std::unique_ptr<SceneInspector> mSceneInspector;             // ptr to scene inspector window
    std::unique_ptr<SnapshotWindow> mSnapshotWindow;             // ptr to snapshot taking/saving window
    std::unique_ptr<StatusBar> mStatusBar;                       // ptr to status bar

    // UI Components
    std::vector<Component*> mComponents;                        // ptrs to all components
    HorizontalDock mBottomDock;                                 // horizontal dock along the bottom of the viewport
    VerticalDock mRightDock;                                    // vertical dock along the right side of the viewport
};

} // namespace moonray_gui_v2
