// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_circle_float.cc

#include "attributes.cc"
#include "ND_circle_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_circle_float, Map)
public:
    ND_circle_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_circle_float mIspc;

RDL2_DSO_CLASS_END(ND_circle_float)

ND_circle_float::ND_circle_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_circle_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_circle_float_getSampleFunc();
}

void
ND_circle_float::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_circle_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_circle_float* me = static_cast<const ND_circle_float*>(self);

    // Get attribute values
    Vec2f centerValue   = evalVec2f(me, centerAttr, tls, state);
    Float radiusValue   = evalFloat(me, radiusAttr, tls, state);
    Vec2f texcoordValue = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);

    // Do procedural calculation
    float dSq = lengthSqr(texcoordValue - centerValue);
    float rSq = radiusValue * radiusValue;
    float outValue = (dSq < rSq) ? 1.0f : 0.0f;
    *sample = Color(outValue);
}

