// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_crosshatch_color3.cc

#include "attributes.cc"
#include "ND_crosshatch_color3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_crosshatch_color3, Map)
public:
    ND_crosshatch_color3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_crosshatch_color3 mIspc;

RDL2_DSO_CLASS_END(ND_crosshatch_color3)

ND_crosshatch_color3::ND_crosshatch_color3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_crosshatch_color3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_crosshatch_color3_getSampleFunc();
}

void
ND_crosshatch_color3::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_crosshatch_color3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_crosshatch_color3* me = static_cast<const ND_crosshatch_color3*>(self);

    // Get attributes
    Vec2f uvtiling  = evalVec2f(me, uvtilingAttr,  tls, state);
    Vec2f uvoffset  = evalVec2f(me, uvoffsetAttr,  tls, state);
    Float thickness = evalFloat(me, thicknessAttr, tls, state);
    Bool  staggered = evalBool (me, staggeredAttr, tls, state);
    Vec2f texcoord  = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);

    // Scale and offset crosshatch pattern so that 1 tile maps to [0,1]x[0,1] in uv space
    Vec2f uv = texcoord * uvtiling - uvoffset;

    // Stagger in u direction if v's integer part is odd
    if (staggered) {
        float vFloor = floor(uv.y);
        int vInt = (int)vFloor;
        if (vInt & 1) uv.x = uv.x + 0.5f;
    }

    // Remap to 45 degrees rotated coordinates with a tilesize of 1/sqrt(2)
    float x = uv.x + uv.y;
    float y = uv.x - uv.y;

    // Find fractional coords with a rotated tile centred on the square gap between the crosshatches
    float xFrac = x - floor(x);
    float yFrac = y - floor(y);

    // Determine axial distances from tile centre
    float ax = scene_rdl2::math::abs(xFrac - 0.5f);
    float ay = scene_rdl2::math::abs(yFrac - 0.5f);

    // Pick greatest distance and threshold it appropriately
    float maxDist = scene_rdl2::math::max(ax, ay);
    float threshold = 0.5f - thickness * 0.70710678f;
    float out = (maxDist > threshold) ? 1.0f : 0.0f;

    // Output result
    *sample = Color(out);
}

