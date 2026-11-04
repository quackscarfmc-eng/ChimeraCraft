// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_combine2_vector2.cc

#include "attributes.cc"
#include "ND_combine2_vector2_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_combine2_vector2, Map)
public:
    ND_combine2_vector2(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_combine2_vector2 mIspc;

RDL2_DSO_CLASS_END(ND_combine2_vector2)

ND_combine2_vector2::ND_combine2_vector2(const SceneClass& sceneClass,
                                         const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_combine2_vector2::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_combine2_vector2_getSampleFunc();
}

void
ND_combine2_vector2::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_combine2_vector2::sample(const Map* self, moonray::shading::TLState *tls,
                            const moonray::shading::State& state, Color* sample)
{
    const ND_combine2_vector2* me = static_cast<const ND_combine2_vector2*>(self);
    const float in1Value = evalFloat(me, in1Attr, tls, state);
    const float in2Value = evalFloat(me, in2Attr, tls, state);

    Vec2f outValue;
    if (me->mIspc.disableMode) {
        outValue.x = 0.f;
        outValue.y = 0.f;
    } else {
        outValue.x = in1Value;
        outValue.y = in2Value;
    }
    *sample = Color(outValue.x, outValue.y, 0.0f);
}
