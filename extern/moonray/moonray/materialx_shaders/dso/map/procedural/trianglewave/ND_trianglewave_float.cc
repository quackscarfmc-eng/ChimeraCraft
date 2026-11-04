// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_trianglewave_float.cc

#include "attributes.cc"
#include "ND_trianglewave_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_trianglewave_float, Map)
public:
    ND_trianglewave_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_trianglewave_float mIspc;

RDL2_DSO_CLASS_END(ND_trianglewave_float)

ND_trianglewave_float::ND_trianglewave_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_trianglewave_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_trianglewave_float_getSampleFunc();
}

void
ND_trianglewave_float::update()
{
}

void
ND_trianglewave_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_trianglewave_float* me = static_cast<const ND_trianglewave_float*>(self);
    float in = evalFloat(me, inAttr, tls, state);
    float t = 2.0f * (in - floor(in));
    float out = (t < 1.0f) ? t : 2.0f-t;
    *sample = Color(out);
}

