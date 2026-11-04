// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file distance.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants:
//  - vector2
//  - vector3
//  - vector4

#define OPERATION distance
#define IN_DEFAULT_FLOAT 0.f

//
// Configure the variants

#if STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_VEC4F

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Map shader
//
#include <moonray/rendering/shading/MapApi.h>

//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<IN_TYPE> in1Attr;
    AttributeKey<IN_TYPE> in2Attr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    in1Attr = sceneClass.declareAttribute<IN_TYPE>("in1", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in2Attr = sceneClass.declareAttribute<IN_TYPE>("in2", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END


//
// end of attributes.cc
//

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::SHADER_NAME mIspc;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update() {}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE in1Value = IN_EVAL(me, in1Attr, tls, state);
    const IN_TYPE in2Value = IN_EVAL(me, in2Attr, tls, state);

    float outValue = distance(in1Value, in2Value);

    *sample = Color(outValue);
}

