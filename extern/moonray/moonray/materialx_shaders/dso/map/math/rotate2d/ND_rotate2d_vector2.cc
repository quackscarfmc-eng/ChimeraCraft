// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_rotate2d_vector2.cc

#include "attributes.cc"
#include "ND_rotate2d_vector2_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_rotate2d_vector2, Map)
public:
    ND_rotate2d_vector2(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_rotate2d_vector2 mIspc;

RDL2_DSO_CLASS_END(ND_rotate2d_vector2)

ND_rotate2d_vector2::ND_rotate2d_vector2(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_rotate2d_vector2::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_rotate2d_vector2_getSampleFunc();
}

void
ND_rotate2d_vector2::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_rotate2d_vector2::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_rotate2d_vector2* me = static_cast<const ND_rotate2d_vector2*>(self);
    const Vec2f inValue = evalVec2f(me, inAttr, tls, state);

    Vec2f outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
        const float amountValue = evalFloat(me, amountAttr, tls, state);
        const float theta = deg2rad(amountValue);

        float sintheta, costheta;
        sincos(theta, &sintheta, &costheta);

        outValue = Vec2f(inValue.x * costheta - inValue.y * sintheta,
                         inValue.x * sintheta + inValue.y * costheta);
    }
    *sample = Color(outValue.x, outValue.y, 0.0f);
}

