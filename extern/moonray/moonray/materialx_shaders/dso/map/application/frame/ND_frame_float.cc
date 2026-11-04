// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_frame_float.cc

#include "attributes.cc"
#include "ND_frame_float_ispc_stubs.h"

#include <moonray/common/mcrt_macros/moonray_static_check.h>
#include <moonray/rendering/shading/MapApi.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_frame_float, Map)
public:
    ND_frame_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_frame_float mIspc;

RDL2_DSO_CLASS_END(ND_frame_float)

ND_frame_float::ND_frame_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_frame_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_frame_float_getSampleFunc();
}

void
ND_frame_float::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));

    // Get the current frame from scene variables
    const scene_rdl2::rdl2::SceneVariables &sv = getSceneClass().getSceneContext()->getSceneVariables();
    mIspc.mFrame = static_cast<float>(sv.get(scene_rdl2::rdl2::SceneVariables::sFrameKey));
}

void
ND_frame_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_frame_float* me = static_cast<const ND_frame_float*>(self);

    if (me->mIspc.mDisableMode) {
        *sample = Color(0.f, 0.f, 0.f);
        return;
    }

    float frameValue = me->mIspc.mFrame;
    *sample = Color(frameValue, frameValue, frameValue);
}
