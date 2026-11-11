// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_combine2_vector4VF.cc

#include "attributes.cc"
#include "ND_combine2_vector4VF_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_combine2_vector4VF, Map)
public:
    ND_combine2_vector4VF(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Vec4f *sample);
    ispc::ND_combine2_vector4VF mIspc;

RDL2_DSO_CLASS_END(ND_combine2_vector4VF)

ND_combine2_vector4VF::ND_combine2_vector4VF(const SceneClass& sceneClass,
                                             const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFuncBool = (SampleFuncBool) ND_combine2_vector4VF::sample;
    mSampleFuncvBool = (SampleFuncvBool) ispc::ND_combine2_vector4VF_getSampleFunc();
}

void
ND_combine2_vector4VF::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_combine2_vector4VF::sample(const Map* self, moonray::shading::TLState *tls,
                              const moonray::shading::State& state, Vec4f* sample)
{
    const ND_combine2_vector4VF* me = static_cast<const ND_combine2_vector4VF*>(self);
    const Vec3f in1Value = evalVec3f(me, in1Attr, tls, state);
    const float in2Value = evalFloat(me, in2Attr, tls, state);

    Vec4f outValue;
    if (me->mIspc.disableMode) {
        outValue.x = 0.f;
        outValue.y = 0.f;
        outValue.z = 0.f;
        outValue.w = 0.f;
    } else {
        outValue.x = in1Value.x;
        outValue.y = in1Value.y;
        outValue.z = in1Value.z;
        outValue.w = in2Value;
    }
    *sample = outValue;
}
