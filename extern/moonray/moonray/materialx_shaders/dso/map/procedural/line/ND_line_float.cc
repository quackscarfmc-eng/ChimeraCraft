// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_line_float.cc

#include "attributes.cc"
#include "ND_line_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_line_float, Map)
public:
    ND_line_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_line_float mIspc;

RDL2_DSO_CLASS_END(ND_line_float)

ND_line_float::ND_line_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_line_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_line_float_getSampleFunc();
}

void
ND_line_float::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_line_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_line_float* me = static_cast<const ND_line_float*>(self);

    // Get attribute values
    const Vec2f texcoord = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);
    const Vec2f center   = evalVec2f(me, centerAttr, tls, state);
    const Vec2f point1   = evalVec2f(me, point1Attr, tls, state);
    const Vec2f point2   = evalVec2f(me, point2Attr, tls, state);
    const float radius   = evalFloat(me, radiusAttr, tls, state);

    // The 1.39 spec says that center gets added to point1 and point2, but we can achieve the same result
    // more cheaply by subtracting it off texcoord
    const Vec2f uv = texcoord - center;

    const Vec2f mid = 0.5f * (point1 + point2);   // midpoint of line
    const Vec2f seg = 0.5f * (point2 - point1);   // segment from midpoint to point2
    const Vec2f pos = uv - mid;                   // position relative to midpoint

    const float pp = dot(pos, pos);               // dot products needed for coordinate transformation
    const float ps = dot(pos, seg);
    const float ss = dot(seg, seg);

    // Transform the point into the local coordinate frame of the line segment, with the origin at its midpoint and the
    // x-axis parallel to the line segment. For improved efficiency, the working coordinates are multiplied through by
    // length(seg), which allows for the removal of divide operations. Also, only the squares of some values are
    // generated, to avoid taking any square roots. Finally, the absolute value of x is used so we can be agnostic to
    // which endpoint we are comparing against, reducing the number of cases we must handle, and the xClamped value
    // effectively selects appropriately between the distance to the endpoint and the distance to the line segment.
    const float x = scene_rdl2::math::abs(ps);
    const float xClamped  = max(x - ss, 0.0f);
    const float y2 = pp * ss - ps * ps;
    const float r2 = radius * radius * ss;
    const float d2 = xClamped * xClamped + y2;

    *sample = Color((r2 > d2) ? 1.0f : 0.0f);
}

