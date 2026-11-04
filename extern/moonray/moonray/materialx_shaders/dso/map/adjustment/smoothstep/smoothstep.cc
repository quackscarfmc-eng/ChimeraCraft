// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file smoothstep.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION smoothstep
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
// Handle inLow and inHigh smoothstep types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define SMOOTHSTEP_TYPE Float
#else
#define SMOOTHSTEP_TYPE IN_TYPE
#endif

#define SMOOTHSTEP_EVAL CONCAT2(eval,SMOOTHSTEP_TYPE)

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define SMOOTHSTEP_LOW_DEFAULT LOW_DEFAULT_FLOAT
#define SMOOTHSTEP_HIGH_DEFAULT HIGH_DEFAULT_FLOAT

#elif STRING_CMP(VARIANT,vector2)
#define SMOOTHSTEP_LOW_DEFAULT Vec2f(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define SMOOTHSTEP_HIGH_DEFAULT Vec2f(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector3)
#define SMOOTHSTEP_LOW_DEFAULT Vec3f(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define SMOOTHSTEP_HIGH_DEFAULT Vec3f(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector4)
#define SMOOTHSTEP_LOW_DEFAULT Vec4f(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define SMOOTHSTEP_HIGH_DEFAULT Vec4f(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color3)
#define SMOOTHSTEP_LOW_DEFAULT Color(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define SMOOTHSTEP_HIGH_DEFAULT Color(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color4)
#define SMOOTHSTEP_LOW_DEFAULT Rgba(LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT,LOW_DEFAULT_FLOAT)
#define SMOOTHSTEP_HIGH_DEFAULT Rgba(HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT,HIGH_DEFAULT_FLOAT)

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
    AttributeKey<SMOOTHSTEP_TYPE> lowAttr;
    AttributeKey<SMOOTHSTEP_TYPE> highAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    lowAttr = sceneClass.declareAttribute<SMOOTHSTEP_TYPE>("low", SMOOTHSTEP_LOW_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    highAttr = sceneClass.declareAttribute<SMOOTHSTEP_TYPE>("high", SMOOTHSTEP_HIGH_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
smoothstep(float inValue,
           float low,
           float high)
{
    // Avoid divide-by-zero
    if (low == high) return 0.0f;

    // Detect direction
    bool inverted = (low > high);
    if (inverted) std::swap(low, high);

    float t = (inValue - low) / (high - low);
    t = scene_rdl2::math::clamp(t, 0.0f, 1.0f);
    float result = t * t * (3.0f - 2.0f * t);

    return inverted ? 1.0f - result : result;
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);
    const SMOOTHSTEP_TYPE low = SMOOTHSTEP_EVAL(me, lowAttr, tls, state);
    const SMOOTHSTEP_TYPE high = SMOOTHSTEP_EVAL(me, highAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = smoothstep(inValue, low, high);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = smoothstep(inValue.r, low.r, high.r);
        outValue.g = smoothstep(inValue.g, low.g, high.g);
        outValue.b = smoothstep(inValue.b, low.b, high.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = smoothstep(inValue.r, low.r, high.r);
        outValue.g = smoothstep(inValue.g, low.g, high.g);
        outValue.b = smoothstep(inValue.b, low.b, high.b);
        outValue.a = smoothstep(inValue.a, low.a, high.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = smoothstep(inValue.x, low.x, high.x);
        outValue.y = smoothstep(inValue.y, low.y, high.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = smoothstep(inValue.x, low.x, high.x);
        outValue.y = smoothstep(inValue.y, low.y, high.y);
        outValue.z = smoothstep(inValue.z, low.z, high.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = smoothstep(inValue.x, low.x, high.x);
        outValue.y = smoothstep(inValue.y, low.y, high.y);
        outValue.z = smoothstep(inValue.z, low.z, high.z);
        outValue.w = smoothstep(inValue.w, low.w, high.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = smoothstep(inValue.r, low, high);
        outValue.g = smoothstep(inValue.g, low, high);
        outValue.b = smoothstep(inValue.b, low, high);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = smoothstep(inValue.r, low, high);
        outValue.g = smoothstep(inValue.g, low, high);
        outValue.b = smoothstep(inValue.b, low, high);
        outValue.a = smoothstep(inValue.a, low, high);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = smoothstep(inValue.x, low, high);
        outValue.y = smoothstep(inValue.y, low, high);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = smoothstep(inValue.x, low, high);
        outValue.y = smoothstep(inValue.y, low, high);
        outValue.z = smoothstep(inValue.z, low, high);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = smoothstep(inValue.x, low, high);
        outValue.y = smoothstep(inValue.y, low, high);
        outValue.z = smoothstep(inValue.z, low, high);
        outValue.w = smoothstep(inValue.w, low, high);
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

