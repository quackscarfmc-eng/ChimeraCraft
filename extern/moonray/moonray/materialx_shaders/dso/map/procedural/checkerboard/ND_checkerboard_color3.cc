// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_checkerboard_color3.cc

#include "attributes.cc"
#include "ND_checkerboard_color3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_checkerboard_color3, Map)
public:
    ND_checkerboard_color3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_checkerboard_color3 mIspc;

RDL2_DSO_CLASS_END(ND_checkerboard_color3)

ND_checkerboard_color3::ND_checkerboard_color3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_checkerboard_color3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_checkerboard_color3_getSampleFunc();
}

void
ND_checkerboard_color3::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_checkerboard_color3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_checkerboard_color3* me = static_cast<const ND_checkerboard_color3*>(self);

    // Get attribute values
    Color color1Value   = evalColor(me, color1Attr,   tls, state);
    Color color2Value   = evalColor(me, color2Attr,   tls, state);
    Vec2f uvtilingValue = evalVec2f(me, uvtilingAttr, tls, state);
    Vec2f uvoffsetValue = evalVec2f(me, uvoffsetAttr, tls, state);
    Vec2f texcoordValue = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);

    // Do procedural calculation
    Vec2f uv = texcoordValue * uvtilingValue - uvoffsetValue;
    int uvXor = (int)floor(uv.x) ^ (int)floor(uv.y);
    *sample = (uvXor & 1) ? color1Value : color2Value;
}

