// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_crossproduct_vector3.cc

#include "attributes.cc"
#include "ND_crossproduct_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_crossproduct_vector3, Map)
public:
    ND_crossproduct_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_crossproduct_vector3 mIspc;

RDL2_DSO_CLASS_END(ND_crossproduct_vector3)

ND_crossproduct_vector3::ND_crossproduct_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_crossproduct_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_crossproduct_vector3_getSampleFunc();
}

void
ND_crossproduct_vector3::update()
{
    mIspc.disableMode = get(disableAttr);
}

void
ND_crossproduct_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_crossproduct_vector3* me = static_cast<const ND_crossproduct_vector3*>(self);

    Vec3f in1Value;
    in1Value = evalVec3f(me, in1Attr, tls, state);

    Vec3f in2Value;
    in2Value = evalVec3f(me, in2Attr, tls, state);

    Vec3f outValue;
    if (me->mIspc.disableMode) {
        outValue = in1Value;
    } else {
        outValue = cross(in1Value, in2Value);
    }
    *sample = Color(outValue.x, outValue.y, outValue.z);
}

