// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_swizzle_color4_color3.cc

#include "attributes.cc"
#include "ND_swizzle_color4_color3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_swizzle_color4_color3, Map)
public:
    ND_swizzle_color4_color3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_swizzle_color4_color3 mIspc;

RDL2_DSO_CLASS_END(ND_swizzle_color4_color3)

ND_swizzle_color4_color3::ND_swizzle_color4_color3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_swizzle_color4_color3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_swizzle_color4_color3_getSampleFunc();
}

void
ND_swizzle_color4_color3::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
    String channels = get(channelsAttr);
    String lookup = "rgba";
    if (channels.length() != 3) {
        mIspc.rChannel = -1;
    } else {
        mIspc.rChannel = lookup.find(channels[0]);
        mIspc.gChannel = lookup.find(channels[1]);
        mIspc.bChannel = lookup.find(channels[2]);
        if ((mIspc.gChannel < 0) || (mIspc.bChannel < 0)) {
            mIspc.rChannel = -1;
        }
    }
}

static float
pickChannel(const Color& input, int channel)
{
    switch (channel) {
      case 0:
        return input.r;
      case 1:
        return input.g;
      case 2:
        return input.b;
    }
}

void
ND_swizzle_color4_color3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_swizzle_color4_color3* me = static_cast<const ND_swizzle_color4_color3*>(self);

    // SdfType is color4f
    // NdrType is color4
    Rgba inValue;
    inValue = evalRgba(me, inAttr, tls, state);

    // SdfType is color3f
    // NdrType is color
    Color outValue;
    if (me->mIspc.disableMode) {
        outValue = Color(0.0f, 0.0f, 0.0f);
    } else if (me->mIspc.rChannel < 0) {
        outValue = Rgba(1.0f, 0.0f, 1.0f,0.0f);
    } else {
        outValue = Rgba(pickChannel(inValue, me->mIspc.rChannel),
                        pickChannel(inValue, me->mIspc.gChannel),
                        pickChannel(inValue, me->mIspc.bChannel),
                        pickChannel(inValue, me->mIspc.aChannel));

    }
    *sample = Color(outValue);
}

