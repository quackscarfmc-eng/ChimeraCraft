// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_roughness_dual.cc

#include "attributes.cc"
#include "ND_roughness_dual_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_roughness_dual, Map)
public:
    ND_roughness_dual(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_roughness_dual mIspc;

RDL2_DSO_CLASS_END(ND_roughness_dual)

ND_roughness_dual::ND_roughness_dual(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_roughness_dual::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_roughness_dual_getSampleFunc();
}

void
ND_roughness_dual::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_roughness_dual::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_roughness_dual* me = static_cast<const ND_roughness_dual*>(self);
    const bool disable = me->mIspc.mDisableMode;

    const Vec2f roughness = evalVec2f(me, roughnessAttr, tls, state);
    const float x = disable ? 0.0f : clamp(roughness.x * roughness.x, sEpsilon, 1.0f);
    const float y = disable ? 0.0f : clamp(roughness.y * roughness.y, sEpsilon, 1.0f);

    *sample = Color(x, y, 0.0f);
}

