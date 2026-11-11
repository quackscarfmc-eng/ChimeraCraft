// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_glossiness_anisotropy.cc

#include "attributes.cc"
#include "ND_glossiness_anisotropy_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_glossiness_anisotropy, Map)
public:
    ND_glossiness_anisotropy(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_glossiness_anisotropy mIspc;

RDL2_DSO_CLASS_END(ND_glossiness_anisotropy)

ND_glossiness_anisotropy::ND_glossiness_anisotropy(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_glossiness_anisotropy::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_glossiness_anisotropy_getSampleFunc();
}

void
ND_glossiness_anisotropy::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_glossiness_anisotropy::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_glossiness_anisotropy* me = static_cast<const ND_glossiness_anisotropy*>(self);

    const float gloss = evalFloat(me, glossinessAttr, tls, state);
    const float roughness = 1.0f - saturate(gloss);
    const float perceptualRoughness = clamp(roughness * roughness, sEpsilon, 1.0f);

    const float anisotropy = evalFloat(me, anisotropyAttr, tls, state);
    Vec2f outValue = Vec2f(roughness, roughness);
    if (me->mIspc.mDisableMode) {
        outValue.x = 0.f;
        outValue.y = 0.f;
    } else if (anisotropy > 0.0f) {
        const float aspect = scene_rdl2::math::sqrt(1.0 - clamp(anisotropy, 0.0f, 0.98f));
        outValue.x = min(perceptualRoughness / aspect, 1.0f);
        outValue.y = perceptualRoughness * aspect;
    }

    *sample = Color(outValue.x, outValue.y, 0.0f);
}

