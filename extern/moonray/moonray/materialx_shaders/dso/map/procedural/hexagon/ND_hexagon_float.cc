// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_hexagon_float.cc

#include "attributes.cc"
#include "ND_hexagon_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_hexagon_float, Map)
public:
    ND_hexagon_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_hexagon_float mIspc;

RDL2_DSO_CLASS_END(ND_hexagon_float)

ND_hexagon_float::ND_hexagon_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_hexagon_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_hexagon_float_getSampleFunc();
}

void
ND_hexagon_float::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_hexagon_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_hexagon_float* me = static_cast<const ND_hexagon_float*>(self);

    // Get attribute values
    Vec2f texcoord = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);
    Vec2f center   = evalVec2f(me, centerAttr, tls, state);
    Float radius   = evalFloat(me, radiusAttr, tls, state);

    // Do procedural calculation
    Vec2f uv = texcoord - center;
    float d0 = scene_rdl2::math::abs(uv.x);                             //   0, 180 degrees
    float d1 = scene_rdl2::math::abs(0.5f * uv.x + 0.8660254f * uv.y);  //  60, 240 degrees
    float d2 = scene_rdl2::math::abs(0.5f * uv.x - 0.8660254f * uv.y);  // 120, 300 degrees
    float d  = scene_rdl2::math::max(d0, d1, d2);
    float out = (d < radius) ? 1.0f : 0.0f;
    *sample = Color(out);
}

