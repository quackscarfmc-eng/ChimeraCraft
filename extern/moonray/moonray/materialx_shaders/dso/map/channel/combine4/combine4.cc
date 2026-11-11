// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file combine4.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// color4, vector4

#define OPERATION combine4
#define IN_DEFAULT_FLOAT 0.0f

//
// Configure the variants
//
#if STRING_CMP(VARIANT,color4)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE TYPE_VEC4F

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define EXTRACT_TYPE IN_TYPE
#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Map shader
//
#include <moonray/rendering/shading/MapApi.h>

#if STRING_CMP(VARIANT,color4)
    #define RETURN_TYPE Rgba
#elif STRING_CMP(VARIANT,vector4)
    #define RETURN_TYPE Vec4f
#endif

//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<Float> in1Attr;
    AttributeKey<Float> in2Attr;
    AttributeKey<Float> in3Attr;
    AttributeKey<Float> in4Attr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    in1Attr = sceneClass.declareAttribute<Float>("in1", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in2Attr = sceneClass.declareAttribute<Float>("in2", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in3Attr = sceneClass.declareAttribute<Float>("in3", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in4Attr = sceneClass.declareAttribute<Float>("in4", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
                       const moonray::shading::State &state, RETURN_TYPE *sample);
    ispc::SHADER_NAME mIspc;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    // it's a Map shader
    // the functions pointers on Map are unions so it doesn't
    // really matter which one is assigned to and the variability
    // is easier to manage with macros with picking one and forcing
    // with a cast
    mSampleFuncBool = (SampleFuncBool) SHADER_NAME::sample;
    mSampleFuncvBool = (SampleFuncvBool) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
SHADER_NAME::sample(const Map * self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const Float in1Value = evalFloat(me, in1Attr, tls, state);
    const Float in2Value = evalFloat(me, in2Attr, tls, state);
    const Float in3Value = evalFloat(me, in3Attr, tls, state);
    const Float in4Value = evalFloat(me, in4Attr, tls, state);

    RETURN_TYPE outValue;
    #if STRING_CMP(VARIANT,color4)
        if (me->mIspc.disableMode) {
            outValue.r = 0.f;
            outValue.g = 0.f;
            outValue.b = 0.f;
            outValue.a = 0.f;
        } else {
            outValue.r = in1Value;
            outValue.g = in2Value;
            outValue.b = in3Value;
            outValue.a = in4Value;
        }
    #elif STRING_CMP(VARIANT,vector4)
        if (me->mIspc.disableMode) {
            outValue.x = 0.f;
            outValue.y = 0.f;
            outValue.z = 0.f;
            outValue.w = 0.f;
        } else {
            outValue.x = in1Value;
            outValue.y = in2Value;
            outValue.z = in3Value;
            outValue.w = in4Value;
        }
    #endif

    *sample = outValue;
}
