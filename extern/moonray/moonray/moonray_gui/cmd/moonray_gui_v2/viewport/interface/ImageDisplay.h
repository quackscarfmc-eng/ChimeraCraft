// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "imgui.h"

#include <scene_rdl2/common/math/Color.h>

#include <GLFW/glfw3.h>     // for GLuint

namespace moonray { namespace rndr { class PathVisualizerManager; }}

namespace moonray_gui_v2 {

class Viewport;

class ImageDisplay {
public:
    ImageDisplay() {}

    // Draw the window to the screen
    void draw(const Viewport* viewport, int availWidth, int availHeight);

    // Get the coordinate for the image pixel 
    // underneath the mouse at the given position
    ImVec2 viewportToImageCoords(int viewportPosX, int viewportPosY) const;

    // Given the coordinates of a pixel in the displayed image, 
    // get the corresponding viewport coordinates. This is necessary 
    // for drawing overlays (like path visualizer lines) in the correct place,
    // since the image itself may be zoomed or panned.
    ImVec2 imageToViewportCoords(int pixelX, int pixelY) const;

    // Get the (unscaled) size of the image
    ImVec2 getImageSize() const { return mImageSize; }

    // Get the (scaled) size of the image
    ImVec2 getScaledSize() const { return ImVec2(mImageSize.x * mZoomScale, mImageSize.y * mZoomScale); }

    // Set the base image size (for resizing in the rdla)
    void setImageSize(int width, int height) { mImageSize = ImVec2(width, height); }

    // Zoom in/out (i.e. increase/decrease the image scale)
    void zoom(int xoffset, int yoffset);

    // Pan the image by the given offset
    void pan(int xoffset, int yoffset);

private:
    // Define the window settings (size, position, style, etc)
    void configureWindow(const int availWidth, const int availHeight);

    /// Draw a line in the image display
    /// @param x1, y1: start point in pixel coordinates
    /// @param x2, y2: end point in pixel coordinates
    /// @param color: color of the line
    /// @param a: alpha value of the line
    /// @param w: width of the line
    /// @param drawEndPoint: whether to draw the end point of the line
    ///
    void drawLine(int x1, int y1, int x2, int y2, const scene_rdl2::math::Color& color, 
                  float a, float w, bool drawEndPoint, bool selected, bool isSample, bool showEndpointsOnly);

    /// Draw all path visualizer lines
    void drawPathVisualizerLines(moonray::rndr::PathVisualizerManager* manager, bool showEndpointsOnly);

    ImVec2 mImageSize {ImVec2(0,0)};        // size of the image
    ImVec2 mPosition  {ImVec2(0,0)};        // position of the image in the viewport

    ImVec2 mPositionOffset {ImVec2(0,0)};   // panning offset
    float mZoomScale {1.0f};                // zoom scale
};

} // end namespace moonray_gui_v2
