// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_reflect_vector3.cc

#include "attributes.cc"
#include "ND_reflect_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_reflect_vector3, Map)
public:
    ND_reflect_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_reflect_vector3 mIspc;

RDL2_DSO_CLASS_END(ND_reflect_vector3)

ND_reflect_vector3::ND_reflect_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_reflect_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_reflect_vector3_getSampleFunc();
}

void
ND_reflect_vector3::update()
{
    mIspc.disableMode = get(disableAttr);
    mIspc.mNormalUseDefault = (!getBinding(normalAttr) && isDefault(normalAttr));
}

void
ND_reflect_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_reflect_vector3* me = static_cast<const ND_reflect_vector3*>(self);

    Vec3f inValue;
    inValue = evalVec3f(me, inAttr, tls, state);

    Vec3f normalValue;
    if (me->mIspc.mNormalUseDefault) {
        normalValue = state.getN();
    } else {
        normalValue = evalVec3f(me, normalAttr, tls, state);
    }

    Vec3f outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
        // Compute reflection: R = I - 2 * (I dot N) * N
        outValue = inValue - 2.0f * dot(inValue, normalValue) * normalValue;
    }
    *sample = Color(outValue.x, outValue.y, outValue.z);
}

