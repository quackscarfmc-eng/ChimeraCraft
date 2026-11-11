// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"
#include "../../GuiTypes.h"
#include "../../RenderGui.h"
#include <mcrt_denoise/denoiser/Denoiser.h>
#include <functional>

namespace moonray_gui_v2 {

/// Creates a stationary, fixed-size status bar at the bottom of the screen with the following info:
///     - Mouse position in pixel coordinates
///     - Current debug mode
///     - Whether fast progressive mode is on or off
///     - Current denoising mode and whether denoising is on or off
///     - Current denoising buffer
///     - Current camera type (orbit or free)
///     - Name of the current render output
///     - A help button that displays hotkeys and app usage information
class StatusBar : public Component {
public:
    StatusBar() : Component(/*isOpen*/ true) {}
    ~StatusBar() override {}

    // Draws the status bar
    void draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset) override;

    // Set a callback function to be called when the help button is clicked
    void setHelpButtonCallback(const std::function<void()>& callback) { mHelpCallback = callback; }

    int getWidth() const override { return -1; }  // -1 means auto-size to full width
    int getHeight() const override { return mHeight; }

private:
    void configureWindow(const ImVec2& dockOffset) const;

    // Get a display-able string representing the current debug mode
    std::string getDebugModeStr(const DebugMode& mode) const;

    // Get a display-able string representing the current fast progressive render mode
    std::string getFastProgressiveModeStr(const bool isFastProgressive, 
                                          const moonray::rndr::FastRenderMode& mode) const;

    // Get a display-able string representing the current denoising mode
    std::string getDenoiseModeStr(const bool denoisingEnabled, const moonray::denoiser::DenoiserMode& mode) const;

    // Get a display-able string representing the current denoise buffer mode
    std::string getDenoiseBufferStr(const bool denoisingEnabled, const DenoisingBufferMode& mode) const;

    // Get a display-able string representing the current camera type
    std::string getCameraTypeStr(const CameraType& type) const;

    // Get a display-able string representing the current render output
    std::string getRenderOutputStr(const std::string& roName) const;
    
    int mHeight {20};               // Height of the bar
    int mWindowPadding {2};

    float mLargeSpace {15};         // large amt of horizontal spacing (to separate statuses)
    float mSmallSpace {5};          // small amt of horizontal spacing (to separate sub-statuses)
    float mButtonSpace {8};         // spacing between help button and other status items

    std::function<void()> mHelpCallback {nullptr};  // callback for help button
};

} // end namespace moonray_gui_v2
