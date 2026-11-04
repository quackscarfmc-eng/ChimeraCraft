// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_bump_vector3.cc

#include "attributes.cc"
#include "ND_bump_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_bump_vector3, Map)
public:
    ND_bump_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_bump_vector3 mIspc;

RDL2_DSO_CLASS_END(ND_bump_vector3)

ND_bump_vector3::ND_bump_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_bump_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_bump_vector3_getSampleFunc();
}

void
ND_bump_vector3::update()
{
    mIspc.normalUseDefault = (!getBinding(normalAttr) && isDefault(normalAttr));
    mIspc.tangentUseDefault = (!getBinding(tangentAttr) && isDefault(tangentAttr));
}

void
ND_bump_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_bump_vector3* me = static_cast<const ND_bump_vector3*>(self);

    /// NOTE: This is a stub only -- MoonRay doesn't 
    /// currently support convolution in maps

    // Stub implementation: pass through the normal
    Vec3f outValue = state.getN();

    *sample = Color(outValue.x, outValue.y, outValue.z);
}
