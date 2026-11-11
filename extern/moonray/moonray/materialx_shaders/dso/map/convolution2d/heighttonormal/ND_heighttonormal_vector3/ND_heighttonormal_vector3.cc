// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_heighttonormal_vector3.cc

#include "attributes.cc"
#include "ND_heighttonormal_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_heighttonormal_vector3, Map)
public:
    ND_heighttonormal_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_heighttonormal_vector3 mIspc;

RDL2_DSO_CLASS_END(ND_heighttonormal_vector3)

ND_heighttonormal_vector3::ND_heighttonormal_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_heighttonormal_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_heighttonormal_vector3_getSampleFunc();
}

void
ND_heighttonormal_vector3::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
ND_heighttonormal_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_heighttonormal_vector3* me = static_cast<const ND_heighttonormal_vector3*>(self);

    /// NOTE: This is a stub only -- MoonRay doesn't 
    /// currently support convolution in maps

    // Stub implementation: pass through the normal
    Vec3f outValue = state.getN();

    *sample = Color(outValue.x, outValue.y, outValue.z);
}
