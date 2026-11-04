// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_place2d_vector2.cc

#include "attributes.cc"
#include "ND_place2d_vector2_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_place2d_vector2, Map)
public:
    enum OperationOrder {
        SRT = 0,
        TRS = 1
    };

    ND_place2d_vector2(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_place2d_vector2 mIspc;

RDL2_DSO_CLASS_END(ND_place2d_vector2)

ND_place2d_vector2::ND_place2d_vector2(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_place2d_vector2::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_place2d_vector2_getSampleFunc();
}

void
ND_place2d_vector2::update()
{
    mIspc.disableMode = get(disableAttr);
}

Vec2f
rotate2d(const Vec2f& uv, float r)
{
    const float theta = deg2rad(r);

    float sintheta, costheta;
    sincos(theta, &sintheta, &costheta);

    return Vec2f(uv.x * costheta - uv.y * sintheta,
                 uv.x * sintheta + uv.y * costheta);
}

Vec2f 
transform_SRT(const Vec2f& uv, const Vec2f& p, const Vec2f& s, float r, const Vec2f& o)
{
    // Performs -pivot, scale, rotate, translate, +pivot
    return rotate2d((uv - p) * s, r) + o + p;
}

Vec2f 
transform_TRS(const Vec2f& uv, const Vec2f& p, const Vec2f& s, float r, const Vec2f& o)
{
    // Performs -pivot, translate, rotate, scale, +pivot
    // which does not introduce texture shear
    return rotate2d(uv - p + o, r) * s + p;
}

void
ND_place2d_vector2::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_place2d_vector2* me = static_cast<const ND_place2d_vector2*>(self);

    Vec2f texcoordValue = evalVec2f(me, texcoordAttr, tls, state);
    Vec2f pivotValue = evalVec2f(me, pivotAttr, tls, state);
    Vec2f scaleValue = evalVec2f(me, scaleAttr, tls, state);
    Float rotateValue = evalFloat(me, rotateAttr, tls, state); // given in degrees
    Vec2f offsetValue = evalVec2f(me, offsetAttr, tls, state);
    Int operationorderValue = evalInt(me, operationorderAttr, tls, state);

    Vec2f outValue;
    if (me->mIspc.disableMode) {
        outValue = texcoordValue;
    } else {
        if (operationorderValue == OperationOrder::SRT) {
            outValue = transform_SRT(texcoordValue, pivotValue, scaleValue, rotateValue, offsetValue);
        } else {
            outValue = transform_TRS(texcoordValue, pivotValue, scaleValue, rotateValue, offsetValue);
        }
    }
    *sample = Color(outValue.x, outValue.y, 0.0f);
}

