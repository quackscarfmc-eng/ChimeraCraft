// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file clamp.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION clamp
#define IN_DEFAULT_FLOAT 0.0f
#define LOW_DEFAULT_FLOAT 0.0f
#define HIGH_DEFAULT_FLOAT 1.0f

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)
#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Handle low and high clamping types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define CLAMP_TYPE Float
#else
#define CLAMP_TYPE IN_TYPE
#endif

#define CLAMP_EVAL CONCAT2(eval,CLAMP_TYPE)

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define CLAMP_LOW_DEFAULT LOW_DEFAULT_FLOAT
#define CLAMP_HIGH_DEFAULT HIGH_DEFAULT_FLOAT

#elif STRING_CMP(VARIANT,vector2)
#define CLAMP_LOW_DEFAULT Vec2f(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define CLAMP_HIGH_DEFAULT Vec2f(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector3)
#define CLAMP_LOW_DEFAULT Vec3f(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define CLAMP_HIGH_DEFAULT Vec3f(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector4)
#define CLAMP_LOW_DEFAULT Vec4f(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define CLAMP_HIGH_DEFAULT Vec4f(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color3)
#define CLAMP_LOW_DEFAULT Color(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define CLAMP_HIGH_DEFAULT Color(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color4)
#define CLAMP_LOW_DEFAULT Rgba(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define CLAMP_HIGH_DEFAULT Rgba(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#endif


#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif


//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<IN_TYPE> inAttr;
    AttributeKey<CLAMP_TYPE> lowAttr;
    AttributeKey<CLAMP_TYPE> highAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    lowAttr = sceneClass.declareAttribute<CLAMP_TYPE>("low", CLAMP_LOW_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    highAttr = sceneClass.declareAttribute<CLAMP_TYPE>("high", CLAMP_HIGH_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
    // the functions pointers on Map are unions so it doesn't
    // really matter which one is assigned to and the variability
    // is easier to manage with macros with picking one and forcing
    // with a cast
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

float
clamp(float inValue, float low, float high)
{
    return inValue < low ? low : (inValue > high ? high : inValue);
}

void
SHADER_NAME::sample(const Map * self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);
    const CLAMP_TYPE low = CLAMP_EVAL(me, lowAttr, tls, state);
    const CLAMP_TYPE high = CLAMP_EVAL(me, highAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = clamp(inValue, low, high);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = clamp(inValue.r, low.r, high.r);
        outValue.g = clamp(inValue.g, low.g, high.g);
        outValue.b = clamp(inValue.b, low.b, high.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = clamp(inValue.r, low.r, high.r);
        outValue.g = clamp(inValue.g, low.g, high.g);
        outValue.b = clamp(inValue.b, low.b, high.b);
        outValue.a = clamp(inValue.a, low.a, high.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = clamp(inValue.x, low.x, high.x);
        outValue.y = clamp(inValue.y, low.y, high.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = clamp(inValue.x, low.x, high.x);
        outValue.y = clamp(inValue.y, low.y, high.y);
        outValue.z = clamp(inValue.z, low.z, high.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = clamp(inValue.x, low.x, high.x);
        outValue.y = clamp(inValue.y, low.y, high.y);
        outValue.z = clamp(inValue.z, low.z, high.z);
        outValue.w = clamp(inValue.w, low.w, high.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = clamp(inValue.r, low, high);
        outValue.g = clamp(inValue.g, low, high);
        outValue.b = clamp(inValue.b, low, high);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = clamp(inValue.r, low, high);
        outValue.g = clamp(inValue.g, low, high);
        outValue.b = clamp(inValue.b, low, high);
        outValue.a = clamp(inValue.a, low, high);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = clamp(inValue.x, low, high);
        outValue.y = clamp(inValue.y, low, high);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = clamp(inValue.x, low, high);
        outValue.y = clamp(inValue.y, low, high);
        outValue.z = clamp(inValue.z, low, high);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = clamp(inValue.x, low, high);
        outValue.y = clamp(inValue.y, low, high);
        outValue.z = clamp(inValue.z, low, high);
        outValue.w = clamp(inValue.w, low, high);
#endif
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(IN_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}

