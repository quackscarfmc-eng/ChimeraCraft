// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_rotate3d_vector3.cc

#include "attributes.cc"
#include "ND_rotate3d_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_rotate3d_vector3, Map)
public:
    ND_rotate3d_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_rotate3d_vector3 mIspc;

RDL2_DSO_CLASS_END(ND_rotate3d_vector3)

ND_rotate3d_vector3::ND_rotate3d_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_rotate3d_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_rotate3d_vector3_getSampleFunc();
}

void
ND_rotate3d_vector3::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_rotate3d_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_rotate3d_vector3* me = static_cast<const ND_rotate3d_vector3*>(self);
    const Vec3f inValue = evalVec3f(me, inAttr, tls, state);

    Vec3f outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
        const Vec3f axis = evalVec3f(me, axisAttr, tls, state);
        const float angle = deg2rad(evalFloat(me, amountAttr, tls, state));
        const Mat4f mat = Mat4f::rotate(axis, angle);

        outValue = transformVector(mat, inValue);
    }
    *sample = Color(outValue.x, outValue.y, outValue.z);
}

