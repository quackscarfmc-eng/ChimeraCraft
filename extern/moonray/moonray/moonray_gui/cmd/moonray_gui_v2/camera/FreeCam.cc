// Copyright 2023-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "FreeCam.h"

#include <moonray/rendering/rndr/RenderContext.h>

// must be between 0 and 1
#define FREECAM_MAX_DAMPENING   0.1f

using namespace scene_rdl2::math;

namespace {

Mat4f
makeCameraMatrix(float yaw, float pitch, float roll, const Vec3f &pos)
{
    Mat4f rotYaw, rotPitch, rotRoll;
    rotYaw.setToRotation(Vec4f(0.0f, 1.0f, 0.0f, 0.0f), yaw);
    rotPitch.setToRotation(Vec4f(1.0f, 0.0f, 0.0f, 0.0f), pitch);
    rotRoll.setToRotation(Vec4f(0.0f, 0.0f, 1.0f, 0.0f), roll);
    Mat4f rotation = rotRoll * rotPitch * rotYaw;

    return rotation * Mat4f::translate(Vec4f(pos.x, pos.y, pos.z, 1.0f));
}

// Print out matrix in lua format so it can be pasted into an rdla file.
void printMatrix(const char *comment, const Mat4f &m)
{
    std::cout << "-- " << comment << "\n"
              << "[\"node xform\"] = Mat4("
              << m.vx.x << ", " << m.vx.y << ", " << m.vx.z << ", " << m.vx.w << ", "
              << m.vy.x << ", " << m.vy.y << ", " << m.vy.z << ", " << m.vy.w << ", "
              << m.vz.x << ", " << m.vz.y << ", " << m.vz.z << ", " << m.vz.w << ", "
              << m.vw.x << ", " << m.vw.y << ", " << m.vw.z << ", " << m.vw.w << "),\n"
              << std::endl;
}

}   // end of anon namespace


namespace moonray_gui_v2 {

// Bitflags that represent current movement state
enum
{
    FREECAM_FORWARD     = 0x0001,
    FREECAM_BACKWARD    = 0x0002,
    FREECAM_LEFT        = 0x0004,
    FREECAM_RIGHT       = 0x0008,
    FREECAM_UP          = 0x0010,
    FREECAM_DOWN        = 0x0020,
    FREECAM_SLOW_DOWN   = 0x0040,
    FREECAM_SPEED_UP    = 0x0080
};

//----------------------------------------------------------------------------

void 
FreeCam::setRenderContext(const moonray::rndr::RenderContext &context)
{
    mRenderContext = &context;
}

Mat4f
FreeCam::resetTransform(const Mat4f &xform, const bool makeDefault)
{
    if (!mInitialTransformSet || makeDefault) {
        mInitialTransform = xform;
        mInitialTransformSet = true;
    }

    mPosition = asVec3(xform.row3());
    mVelocity = Vec3f(zero);

    Vec3f viewDir = -normalize(asVec3(xform.row2()));

    mYaw = 0.0f;
    if (viewDir.x * viewDir.x + viewDir.z * viewDir.z > 0.00001f) {
        mYaw = scene_rdl2::math::atan2(-viewDir.x, -viewDir.z);
    }

    // We aren't extracting the entire range of possible pitches here, just the
    // ones which the freecam can natively handle. Because of this, not all camera
    // orientations are supported.
    mPitch = scene_rdl2::math::asin(viewDir.y);

    // Compute a matrix which only contains the roll so we can extract it out.
    Mat4f noRoll = makeCameraMatrix(mYaw, mPitch, 0.0f, Vec3f(0.0f, 0.0f, 0.0f));
    Mat4f rollOnly = xform * noRoll.transposed();
    Vec3f xAxis = normalize(asVec3(rollOnly.row0()));
    mRoll = scene_rdl2::math::atan2(xAxis.y, xAxis.x);

    mInputState = 0;
    mMouseMode = NONE; 
    mMouseX = mMouseY = 0;    
    mMouseDeltaX = mMouseDeltaY = 0;

    return makeCameraMatrix(mYaw, mPitch, mRoll, mPosition);
}

Mat4f
FreeCam::update(const float dt)
{
    // Compute some amount to change our current velocity.
    Vec3f deltaVelocity = Vec3f(zero);
    const float movement = mSpeed * 0.5f;

    // Process keyboard input.
    if (mInputState & FREECAM_FORWARD) {
        deltaVelocity += Vec3f(0.0f, 0.0f, -movement);
    }
    if (mInputState & FREECAM_BACKWARD) {
        deltaVelocity += Vec3f(0.0f, 0.0f, movement);
    }
    if (mInputState & FREECAM_LEFT) {
        deltaVelocity += Vec3f(-movement, 0.0f, 0.0f);
    }
    if (mInputState & FREECAM_RIGHT) {
        deltaVelocity += Vec3f(movement, 0.0f, 0.0f);
    }
    if (mInputState & FREECAM_UP) {
        deltaVelocity += Vec3f(0.0f, movement, 0.0f);
    }
    if (mInputState & FREECAM_DOWN) {
        deltaVelocity += Vec3f(0.0f, -movement, 0.0f);
    }
    if (mInputState & FREECAM_SLOW_DOWN) {
        mSpeed += -mSpeed * dt;
    }
    if (mInputState & FREECAM_SPEED_UP) {
        mSpeed += mSpeed * dt;
    }

    // Update the camera angles by the rotation amounts (ignore dt for this
    // since it should be instant).
    if (mMouseMode == ROTATE) {

        // rotate mouse movement by roll before updating yaw and pitch
        float c, s;
        sincos(-mRoll, &s, &c);

        const float dx = float(mMouseDeltaX) * c - float(mMouseDeltaY) * s;
        const float dy = float(mMouseDeltaY) * c + float(mMouseDeltaX) * s;

        mYaw -= dx * mMouseSensitivity;
        mPitch -= dy * mMouseSensitivity;

    } else if (mMouseMode == ROLL) {
        mRoll += float(mMouseDeltaX) * mMouseSensitivity;
    }
    mMouseDeltaX = mMouseDeltaY = 0;

    // Clip camera pitch to prevent Gimbal Lock.
    const float halfPi = sHalfPi;
    mPitch = clamp(mPitch, -halfPi, halfPi);

    // Transform deltaVelocity into current camera coordinate system.
    Mat4f rotation = makeCameraMatrix(mYaw, mPitch, mRoll, zero);
    deltaVelocity = transform3x3(rotation, deltaVelocity);

    mVelocity += deltaVelocity;

    // Scale back velocity to mSpeed if too big.
    const float len = mVelocity.length();
    if (len > mSpeed) {
        mVelocity *= (mSpeed / len);
    }

    // Integrate position.
    mPosition += mVelocity * dt;

    // Apply dampening to velocity.
    mVelocity *= min(mDampening * dt, FREECAM_MAX_DAMPENING);

    return makeCameraMatrix(mYaw, mPitch, mRoll, mPosition);
}

bool
FreeCam::processKeyPressEvent(GLFWwindow* window, const Action action)
{
    switch (action) {
    case ACTION_CAM_FORWARD:        mInputState |= FREECAM_FORWARD;      return true;
    case ACTION_CAM_BACKWARD:       mInputState |= FREECAM_BACKWARD;     return true;
    case ACTION_CAM_LEFT:           mInputState |= FREECAM_LEFT;         return true;
    case ACTION_CAM_RIGHT:          mInputState |= FREECAM_RIGHT;        return true;
    case ACTION_CAM_UP:             mInputState |= FREECAM_UP;           return true;
    case ACTION_CAM_DOWN:           mInputState |= FREECAM_DOWN;         return true;
    case ACTION_CAM_SLOW_DOWN:      mInputState |= FREECAM_SLOW_DOWN;    return true;
    case ACTION_CAM_SPEED_UP:       mInputState |= FREECAM_SPEED_UP;     return true;
    case ACTION_CAM_PRINT_MATRICES: printCameraMatrices();               return true;
    case ACTION_CAM_SET_UP_VECTOR:  mRoll = 0.0f;                        return true;
    case ACTION_CAM_RESET:          resetCamera();                       return true;
    case ACTION_CAM_FRAME_SCENE:    frameScene();                        return true;
    case ACTION_CAM_ROTATE:         mMouseMode = ROTATE;                 break;
    case ACTION_CAM_ROLL:           mMouseMode = ROLL;                   break;
    default:                        mMouseMode = NONE;
    }

    if (mMouseMode != NONE) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mMouseX = static_cast<int>(xpos);
        mMouseY = static_cast<int>(ypos);
        mMouseDeltaX = mMouseDeltaY = 0;
        return true;
    }

    return false;
}

bool
FreeCam::processKeyReleaseEvent(GLFWwindow* window, const Action action)
{
    mMouseMode = NONE;

    switch (action) {
        case ACTION_CAM_FORWARD:        mInputState &= ~FREECAM_FORWARD;    return true;
        case ACTION_CAM_BACKWARD:       mInputState &= ~FREECAM_BACKWARD;   return true;
        case ACTION_CAM_LEFT:           mInputState &= ~FREECAM_LEFT;       return true;
        case ACTION_CAM_RIGHT:          mInputState &= ~FREECAM_RIGHT;      return true;
        case ACTION_CAM_UP:             mInputState &= ~FREECAM_UP;         return true;
        case ACTION_CAM_DOWN:           mInputState &= ~FREECAM_DOWN;       return true;
        case ACTION_CAM_SLOW_DOWN:      mInputState &= ~FREECAM_SLOW_DOWN;  return true;
        case ACTION_CAM_SPEED_UP:       mInputState &= ~FREECAM_SPEED_UP;   return true;
    }
    return false;
}

bool
FreeCam::processMouseMoveEvent(const double xpos, const double ypos)
{
   if (mMouseMode == ROTATE || mMouseMode == ROLL) {
       mMouseDeltaX += static_cast<int>(xpos - mMouseX);
       mMouseDeltaY += static_cast<int>(ypos - mMouseY);
       mMouseX = static_cast<int>(xpos);
       mMouseY = static_cast<int>(ypos);
       return true;
   }
   return false;
}

void
FreeCam::clearMovementState()
{
    mVelocity = Vec3f(0.0f, 0.0f, 0.0f);
    mInputState = 0;
    mMouseMode = NONE;
    mMouseX = 0;
    mMouseY = 0;
}

void
FreeCam::resetCamera()
{
    if (mInitialTransformSet) {
        clearMovementState();
        resetTransform(mInitialTransform, false);
    }
}

void
FreeCam::frameScene()
{
    MNRY_ASSERT(mRenderContext);
    if (!mRenderContext) {
        return;
    }
    const scene_rdl2::math::BBox3f sceneBounds = mRenderContext->getSceneBoundsWorld();

    const float sceneSize = sceneBounds.size().length();
    if (sceneSize < scene_rdl2::math::sEpsilon) {
        return; // Avoid framing if the scene is too small or degenerate
    }

    const scene_rdl2::math::Vec3f& viewDir = NavigationCam::FRAME_CAM_VIEW; // already normalized

    // Then, position the camera so that we can see the full scene.
    const float focusDistance = sceneSize;
    const scene_rdl2::math::Vec3f sceneCenter = center(sceneBounds);
    mPosition = sceneCenter - viewDir * focusDistance;

    mYaw = scene_rdl2::math::atan2(-viewDir.x, -viewDir.z);
    mPitch = scene_rdl2::math::asin(viewDir.y);
    mRoll = 0.0f; // default to upright
}

void
FreeCam::printCameraMatrices() const
{
    const Mat4f fullMat = makeCameraMatrix(mYaw, mPitch, mRoll, mPosition);
    const Mat4f zeroPitchMat = makeCameraMatrix(mYaw, 0.0f, 0.0f, mPosition);

    printMatrix("Full matrix containing rotation and position.", fullMat);
    printMatrix("Matrix containing world xz rotation and position.", zeroPitchMat);
}

//----------------------------------------------------------------------------

} // namespace moonray_gui_v2

