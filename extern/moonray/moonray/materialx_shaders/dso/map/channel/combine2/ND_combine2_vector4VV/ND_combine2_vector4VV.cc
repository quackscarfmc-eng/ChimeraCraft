// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_combine2_vector4VV.cc

#include "attributes.cc"
#include "ND_combine2_vector4VV_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_combine2_vector4VV, Map)
public:
    ND_combine2_vector4VV(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Vec4f *sample);
    ispc::ND_combine2_vector4VV mIspc;

RDL2_DSO_CLASS_END(ND_combine2_vector4VV)

ND_combine2_vector4VV::ND_combine2_vector4VV(const SceneClass& sceneClass,
                                             const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFuncBool = (SampleFuncBool) ND_combine2_vector4VV::sample;
    mSampleFuncvBool = (SampleFuncvBool) ispc::ND_combine2_vector4VV_getSampleFunc();
}

void
ND_combine2_vector4VV::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_combine2_vector4VV::sample(const Map* self, moonray::shading::TLState *tls,
                              const moonray::shading::State& state, Vec4f* sample)
{
    const ND_combine2_vector4VV* me = static_cast<const ND_combine2_vector4VV*>(self);
    const Vec2f in1Value = evalVec2f(me, in1Attr, tls, state);
    const Vec2f in2Value = evalVec2f(me, in2Attr, tls, state);

    Vec4f outValue;
    if (me->mIspc.disableMode) {
        outValue.x = 0.f;
        outValue.y = 0.f;
        outValue.z = 0.f;
        outValue.w = 0.f;
    } else {
        outValue.x = in1Value.x;
        outValue.y = in1Value.y;
        outValue.z = in2Value.x;
        outValue.w = in2Value.y;
    }
    *sample = outValue;
}
