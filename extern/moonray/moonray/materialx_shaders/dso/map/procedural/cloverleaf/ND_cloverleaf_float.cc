// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_cloverleaf_float.cc

#include "attributes.cc"
#include "ND_cloverleaf_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_cloverleaf_float, Map)
public:
    ND_cloverleaf_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_cloverleaf_float mIspc;

RDL2_DSO_CLASS_END(ND_cloverleaf_float)

ND_cloverleaf_float::ND_cloverleaf_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_cloverleaf_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_cloverleaf_float_getSampleFunc();
}

void
ND_cloverleaf_float::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_cloverleaf_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_cloverleaf_float* me = static_cast<const ND_cloverleaf_float*>(self);

    // Get attribute values
    const Vec2f texcoord = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);
    const Vec2f center   = evalVec2f(me, centerAttr, tls, state);
    const float radius   = evalFloat(me, radiusAttr, tls, state);

    // Do procedural calculation
    // Note: we halve the value of center because the spec says it's "2x the coordinate of the center".
    // Also we use abs and max to exploit the 8-fold symmetry of the cloverleaf, and do some algebraic
    // reduction on the resulting test to improve efficiency.
    const Vec2f uv = abs(texcoord - 0.5f * center);
    *sample = Color((lengthSqr(uv) < radius * max(uv.x, uv.y)) ? 1.0f : 0.0f);
}

