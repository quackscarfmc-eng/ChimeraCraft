// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_refract_vector3.cc

#include "attributes.cc"
#include "ND_refract_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_refract_vector3, Map)
public:
    ND_refract_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_refract_vector3 mIspc;

RDL2_DSO_CLASS_END(ND_refract_vector3)

ND_refract_vector3::ND_refract_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_refract_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_refract_vector3_getSampleFunc();
}

void
ND_refract_vector3::update()
{
    mIspc.disableMode = get(disableAttr);
    mIspc.mNormalUseDefault = (!getBinding(normalAttr) && isDefault(normalAttr));
}

void
ND_refract_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_refract_vector3* me = static_cast<const ND_refract_vector3*>(self);

    Vec3f inValue;
    inValue = evalVec3f(me, inAttr, tls, state);

    Vec3f normalValue;
    if (me->mIspc.mNormalUseDefault) {
        normalValue = state.getN();
    } else {
        normalValue = evalVec3f(me, normalAttr, tls, state);
    }

    float iorValue;
    iorValue = evalFloat(me, iorAttr, tls, state);

    Vec3f outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
        // Compute refraction using Snell's law
        // R = eta * I - (eta * cos(theta_i) + cos(theta_t)) * N
        // where eta = 1/ior (ratio of indices of refraction)
        // cos(theta_i) = -dot(N, I)
        // cos(theta_t) = sqrt(1 - eta^2 * (1 - cos(theta_i)^2))
        
        float eta = 1.0f / iorValue;
        float cosI = -dot(normalValue, inValue);
        float sinT2 = eta * eta * (1.0f - cosI * cosI);
        
        // Check for total internal reflection
        if (sinT2 > 1.0f) {
            // Total internal reflection - return reflection
            outValue = inValue - 2.0f * dot(inValue, normalValue) * normalValue;
        } else {
            float cosT = sqrtf(1.0f - sinT2);
            outValue = eta * inValue + (eta * cosI - cosT) * normalValue;
        }
    }
    *sample = Color(outValue.x, outValue.y, outValue.z);
}
