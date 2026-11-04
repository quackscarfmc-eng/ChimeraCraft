// Copyright 2023-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../GuiTypes.h"

#include <scene_rdl2/common/math/Mat4.h>

#include <GLFW/glfw3.h>

namespace moonray { namespace rndr { class RenderContext; } }

namespace moonray_gui_v2 {

///
/// Pure virtual base class which further navigation models may be implemented
/// on top of.
///
class NavigationCam
{
public:
    // The default camera view vector for framing the scene.
    // Chosen arbitrarily to be a good angle for looking at the scene when framed.
    // Note that the vector is only approximately normalized, which is sufficient for our purposes.
    inline static const scene_rdl2::math::Vec3f FRAME_CAM_VIEW{0.57735f, -0.57735f, 0.57735f};

                         NavigationCam() {}
    virtual             ~NavigationCam() {}

    // Certain types of camera may want to intersect with the scene, in which
    // case they'll need more information about the scene. This function does
    // nothing by default.
    virtual void        setRenderContext(const moonray::rndr::RenderContext &context) {}

    // If this camera model imposes any constraints on the input matrix, then
    // the constrained matrix is returned, otherwise the output will equal in 
    // input.
    // If makeDefault is set to true then this xform is designated a the new
    // default transform when/if the camera is reset.
    virtual scene_rdl2::math::Mat4f resetTransform(const scene_rdl2::math::Mat4f &xform, const bool makeDefault) = 0;

    // Returns the latest camera matrix.
    virtual scene_rdl2::math::Mat4f update(const float dt) = 0;

    /// Returns true if the input was used, false if ignored.
    virtual bool        processKeyPressEvent(GLFWwindow* window, const Action action) { return false; }
    virtual bool        processKeyReleaseEvent(GLFWwindow* window, const Action action) { return false; }
    virtual bool        processMouseMoveEvent(const double xpos, const double ypos) { return false; }

    virtual void        clearMovementState() {};
};

} // namespace moonray_gui_v2


