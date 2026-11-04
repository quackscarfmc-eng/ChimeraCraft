// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_combine2_color4CF.cc

#include "attributes.cc"
#include "ND_combine2_color4CF_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_combine2_color4CF, Map)
public:
    ND_combine2_color4CF(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Rgba *sample);
    ispc::ND_combine2_color4CF mIspc;

RDL2_DSO_CLASS_END(ND_combine2_color4CF)

ND_combine2_color4CF::ND_combine2_color4CF(const SceneClass& sceneClass,
                                           const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFuncBool = (SampleFuncBool) ND_combine2_color4CF::sample;
    mSampleFuncvBool = (SampleFuncvBool) ispc::ND_combine2_color4CF_getSampleFunc();
}

void
ND_combine2_color4CF::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_combine2_color4CF::sample(const Map* self, moonray::shading::TLState *tls,
                             const moonray::shading::State& state, Rgba* sample)
{
    const ND_combine2_color4CF* me = static_cast<const ND_combine2_color4CF*>(self);
    const Color in1Value = evalColor(me, in1Attr, tls, state);
    const float in2Value = evalFloat(me, in2Attr, tls, state);

    Rgba outValue;
    if (me->mIspc.disableMode) {
        outValue.r = 0.f;
        outValue.g = 0.f;
        outValue.b = 0.f;
        outValue.a = 0.f;
    } else {
        outValue.r = in1Value.r;
        outValue.g = in1Value.g;
        outValue.b = in1Value.b;
        outValue.a = in2Value;
    }
    *sample = outValue;
}
