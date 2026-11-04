// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_grid_color3.cc

#include "attributes.cc"
#include "ND_grid_color3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_grid_color3, Map)
public:
    ND_grid_color3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_grid_color3 mIspc;

RDL2_DSO_CLASS_END(ND_grid_color3)

ND_grid_color3::ND_grid_color3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_grid_color3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_grid_color3_getSampleFunc();
}

void
ND_grid_color3::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_grid_color3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_grid_color3* me = static_cast<const ND_grid_color3*>(self);

    // Get attribute values
    Vec2f texcoord  = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);
    Vec2f uvtiling  = evalVec2f(me, uvtilingAttr, tls, state);
    Vec2f uvoffset  = evalVec2f(me, uvoffsetAttr, tls, state);
    Float thickness = evalFloat(me, thicknessAttr, tls, state);
    Bool  staggered = evalBool (me, staggeredAttr, tls, state);

    // Scale and offset crosshatch pattern so that 1 tile maps to [0,1]x[0,1] in uv space
    Vec2f uv = texcoord * uvtiling - uvoffset;

    // Stagger in u direction if v's integer part is odd
    if (staggered) {
        float vFloor = floor(uv.y);
        int vInt = (int)vFloor;
        if (vInt & 1) uv.x = uv.x + 0.5f;
    }

    // Find fractional coords
    float uFrac = uv.x - floor(uv.x);
    float vFrac = uv.y - floor(uv.y);

    // Determine axial distances from tile centre
    float au = scene_rdl2::math::abs(uFrac - 0.5f);
    float av = scene_rdl2::math::abs(vFrac - 0.5f);

    // Pick greatest distance and threshold it appropriately
    float maxDist = scene_rdl2::math::max(au, av);
    float threshold = 0.5f - 0.5f * thickness;
    float out = (maxDist > threshold) ? 1.0f : 0.0f;

    // Output result
    *sample = Color(out);
}

