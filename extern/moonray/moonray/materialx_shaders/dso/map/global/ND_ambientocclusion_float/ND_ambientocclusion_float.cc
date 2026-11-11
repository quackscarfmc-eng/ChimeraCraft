// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_ambientocclusion_float.cc

#include "attributes.cc"
#include "ND_ambientocclusion_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_ambientocclusion_float, Map)
public:
    ND_ambientocclusion_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_ambientocclusion_float mIspc;

RDL2_DSO_CLASS_END(ND_ambientocclusion_float)

ND_ambientocclusion_float::ND_ambientocclusion_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_ambientocclusion_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_ambientocclusion_float_getSampleFunc();
}

void
ND_ambientocclusion_float::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_ambientocclusion_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_ambientocclusion_float* me = static_cast<const ND_ambientocclusion_float*>(self);

    /// NOTE: This is a stub only -- MoonRay doesn't 
    /// currently support ray tracing in maps

    *sample = Color(1.f, 1.f, 1.f);
}
