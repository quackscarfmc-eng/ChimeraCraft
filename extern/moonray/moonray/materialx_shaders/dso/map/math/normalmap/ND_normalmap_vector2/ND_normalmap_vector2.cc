// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_normalmap_vector2.cc

#include "attributes.cc"
#include "ND_normalmap_vector2_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_normalmap_vector2, Map)
public:
    ND_normalmap_vector2(SceneClass const &sceneClass, std::string const &name);
    void update() override;

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);

    ispc::ND_normalmap_vector2 mIspc;

RDL2_DSO_CLASS_END(ND_normalmap_vector2)

ND_normalmap_vector2::ND_normalmap_vector2(
    const SceneClass& sceneClass,
    const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_normalmap_vector2::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_normalmap_vector2_getSampleFunc();
}

void
ND_normalmap_vector2::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
    mIspc.mNormalUseDefault = (!getBinding(normalAttr) && isDefault(normalAttr));
    mIspc.mTangentUseDefault = (!getBinding(tangentAttr) && isDefault(tangentAttr));
    mIspc.mBitangentUseDefault = (!getBinding(bitangentAttr) && isDefault(bitangentAttr));
}

void
ND_normalmap_vector2::sample(const Map* self,
                                   moonray::shading::TLState *tls,
                             const moonray::shading::State& state,
                                   Color* sample)
{
    const ND_normalmap_vector2* me = static_cast<const ND_normalmap_vector2*>(self);

    Vec3f inValue = evalVec3f(me, inAttr, tls, state);

    if (me->mIspc.mDisableMode) {
        *sample = Color(inValue.x, inValue.y, inValue.z);
        return;
    }

    if (dot(inValue, inValue) < sEpsilon) inValue = Vec3f(0.5f, 0.5f, 1.f);

    const Vec2f scaleValue = evalVec2f(me, scaleAttr, tls, state);

    Vec3f N;
    if (me->mIspc.mNormalUseDefault) {
        N = normalize(state.getN());
    } else {
        N = evalVec3f(me, normalAttr, tls, state);
    }

    Vec3f T;
    if (me->mIspc.mTangentUseDefault) {
        T = normalize(state.getdPds());
    } else {
        T = evalVec3f(me, tangentAttr, tls, state);
    }

    Vec3f B;
    if (me->mIspc.mBitangentUseDefault) {
        B = normalize(cross(T, N));
    } else {
        B = evalVec3f(me, bitangentAttr, tls, state);
    }

    inValue = 2.f * inValue - Vec3f(1.f);
    Vec3f outValue = T * scaleValue.x * inValue.x +
                     B * scaleValue.y * inValue.y +
                     N * inValue.z;

    outValue = normalize(outValue);

    *sample = Color(outValue.x, outValue.y, outValue.z);
}
