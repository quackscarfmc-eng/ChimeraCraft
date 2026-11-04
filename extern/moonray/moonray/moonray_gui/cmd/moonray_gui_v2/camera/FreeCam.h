// Copyright 2023-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "NavigationCam.h"

namespace moonray_gui_v2 {

class FreeCam : public NavigationCam
{
public:
    FreeCam() {};
    ~FreeCam() {};

    /// Returns a matrix with only pitch and yaw (no roll).
    scene_rdl2::math::Mat4f  resetTransform(const scene_rdl2::math::Mat4f &xform, const bool makeDefault) override;

    scene_rdl2::math::Mat4f  update(const float dt) override;

    void                setRenderContext(const moonray::rndr::RenderContext &context) override;

    /// Returns true if the input was used, false if ignored.
    bool                processKeyPressEvent(GLFWwindow* window, const Action action) override;
    bool                processKeyReleaseEvent(GLFWwindow* window, const Action action) override;
    bool                processMouseMoveEvent(const double xpos, const double ypos) override;

    void                clearMovementState() override;

private:        
    enum MouseMode
    {
        NONE,
        ROTATE,
        ROLL,
    };

    // Reset camera to its initial state
    void                resetCamera();

    // Frame the scene by adjusting the camera position and orientation to fit the entire scene within the view.
    // This is a best effort based on the scene bounds and may not account for all camera projection 
    // parameters (e.g. FOV, aspect ratio, near plane, etc).
    void                frameScene();

    void                printCameraMatrices() const;

    const moonray::rndr::RenderContext *mRenderContext {nullptr};

    scene_rdl2::math::Vec3f  mPosition {0.0f, 0.0f, 0.0f};
    scene_rdl2::math::Vec3f  mVelocity {0.0f, 0.0f, 0.0f};
    float               mYaw {0.0f};
    float               mPitch {0.0f};
    float               mRoll {0.0f};
    float               mSpeed {10.0f};
    float               mDampening {1.0f};          /// the amount by which mVelocity is dampened each update
    float               mMouseSensitivity {0.004f}; /// multiplier for mouse movement
    uint32_t            mInputState {0};            /// bitfield representing current movement state
    MouseMode           mMouseMode {NONE};          /// current mouse interaction mode (ROTATE, ROLL, NONE)
    int                 mMouseX {0};                /// last mouse x position
    int                 mMouseY {0};                /// last mouse y position
    int                 mMouseDeltaX {0};           /// change in mouse x since last update
    int                 mMouseDeltaY {0};           /// change in mouse y since last update

    bool                mInitialTransformSet {false};
    scene_rdl2::math::Mat4f  mInitialTransform;
};

} // namespace moonray_gui_v2

