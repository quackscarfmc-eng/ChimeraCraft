// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file range.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION range
#define IN_DEFAULT_FLOAT 0.0f
#define INLOW_DEFAULT_FLOAT 0.0f
#define INHIGH_DEFAULT_FLOAT 1.0f
#define OUTLOW_DEFAULT_FLOAT 0.0f
#define OUTHIGH_DEFAULT_FLOAT 1.0f
#define GAMMA_DEFAULT_FLOAT 1.0f
#define DOCLAMP_DEFAULT_BOOL false

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
// Handle inLow and inHigh range types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define RANGE_TYPE Float
#else
#define RANGE_TYPE IN_TYPE
#endif

#define RANGE_EVAL CONCAT2(eval,RANGE_TYPE)

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define RANGE_INLOW_DEFAULT INLOW_DEFAULT_FLOAT
#define RANGE_INHIGH_DEFAULT INHIGH_DEFAULT_FLOAT
#define RANGE_OUTLOW_DEFAULT OUTLOW_DEFAULT_FLOAT
#define RANGE_OUTHIGH_DEFAULT OUTHIGH_DEFAULT_FLOAT
#define RANGE_GAMMA_DEFAULT GAMMA_DEFAULT_FLOAT
#define RANGE_DOCLAMP_DEFAULT DOCLAMP_DEFAULT_BOOL

#elif STRING_CMP(VARIANT,vector2)
#define RANGE_INLOW_DEFAULT Vec2f(INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT)
#define RANGE_INHIGH_DEFAULT Vec2f(INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT)
#define RANGE_OUTLOW_DEFAULT Vec2f(OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT)
#define RANGE_OUTHIGH_DEFAULT Vec2f(OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT)
#define RANGE_GAMMA_DEFAULT Vec2f(GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector3)
#define RANGE_INLOW_DEFAULT Vec3f(INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT)
#define RANGE_INHIGH_DEFAULT Vec3f(INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT)
#define RANGE_OUTLOW_DEFAULT Vec3f(OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT)
#define RANGE_OUTHIGH_DEFAULT Vec3f(OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT)
#define RANGE_GAMMA_DEFAULT Vec3f(GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector4)
#define RANGE_INLOW_DEFAULT Vec4f(INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT)
#define RANGE_INHIGH_DEFAULT Vec4f(INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT)
#define RANGE_OUTLOW_DEFAULT Vec4f(OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT)
#define RANGE_OUTHIGH_DEFAULT Vec4f(OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT)
#define RANGE_GAMMA_DEFAULT Vec4f(GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color3)
#define RANGE_INLOW_DEFAULT Color(INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT)
#define RANGE_INHIGH_DEFAULT Color(INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT)
#define RANGE_OUTLOW_DEFAULT Color(OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT)
#define RANGE_OUTHIGH_DEFAULT Color(OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT)
#define RANGE_GAMMA_DEFAULT Color(GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color4)
#define RANGE_INLOW_DEFAULT Rgba(INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT,INLOW_DEFAULT_FLOAT)
#define RANGE_INHIGH_DEFAULT Rgba(INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT,INHIGH_DEFAULT_FLOAT)
#define RANGE_OUTLOW_DEFAULT Rgba(OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT,OUTLOW_DEFAULT_FLOAT)
#define RANGE_OUTHIGH_DEFAULT Rgba(OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT,OUTHIGH_DEFAULT_FLOAT)
#define RANGE_GAMMA_DEFAULT Rgba(GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT,GAMMA_DEFAULT_FLOAT)

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
    AttributeKey<RANGE_TYPE> inLowAttr;
    AttributeKey<RANGE_TYPE> inHighAttr;
    AttributeKey<RANGE_TYPE> outLowAttr;
    AttributeKey<RANGE_TYPE> outHighAttr;
    AttributeKey<RANGE_TYPE> gammaAttr;
    AttributeKey<Bool> doClampAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    inLowAttr = sceneClass.declareAttribute<RANGE_TYPE>("inlow", RANGE_INLOW_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    inHighAttr = sceneClass.declareAttribute<RANGE_TYPE>("inhigh", RANGE_INHIGH_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    outLowAttr = sceneClass.declareAttribute<RANGE_TYPE>("outlow", RANGE_OUTLOW_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    outHighAttr = sceneClass.declareAttribute<RANGE_TYPE>("outhigh", RANGE_OUTHIGH_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    gammaAttr = sceneClass.declareAttribute<RANGE_TYPE>("gamma", RANGE_GAMMA_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    doClampAttr = sceneClass.declareAttribute<Bool>("doclamp", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
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
    mIspc.doClamp = (get(doClampAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

float
range(float inValue,
      float inLow,
      float inHigh,
      float outLow,
      float outHigh,
      float gamma,
      bool doClamp)
{
    // Avoid division by zero
    if (inHigh == inLow) {
        return outLow;
    }

    // Normalize input to 0–1 remap
    float t = (inValue - inLow) / (inHigh - inLow);

    // Apply gamma correction
    if (gamma != 1.0f && t > 0.0f) {
        t = scene_rdl2::math::pow(t, 1.0f / gamma);
    }

    // Clamp if requested
    if (doClamp) {
        t = clamp(t, 0.0f, 1.0f);
    }

    // Interpolate to output remap
    return outLow + t * (outHigh - outLow);
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);
    const RANGE_TYPE inLow = RANGE_EVAL(me, inLowAttr, tls, state);
    const RANGE_TYPE inHigh = RANGE_EVAL(me, inHighAttr, tls, state);
    const RANGE_TYPE outLow = RANGE_EVAL(me, outLowAttr, tls, state);
    const RANGE_TYPE outHigh = RANGE_EVAL(me, outHighAttr, tls, state);
    const RANGE_TYPE gamma = RANGE_EVAL(me, gammaAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = range(inValue, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = range(inValue.r, inLow.r, inHigh.r, outLow.r, outHigh.r, gamma.r, me->mIspc.doClamp);
        outValue.g = range(inValue.g, inLow.g, inHigh.g, outLow.g, outHigh.g, gamma.g, me->mIspc.doClamp);
        outValue.b = range(inValue.b, inLow.b, inHigh.b, outLow.b, outHigh.b, gamma.b, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = range(inValue.r, inLow.r, inHigh.r, outLow.r, outHigh.r, gamma.r, me->mIspc.doClamp);
        outValue.g = range(inValue.g, inLow.g, inHigh.g, outLow.g, outHigh.g, gamma.g, me->mIspc.doClamp);
        outValue.b = range(inValue.b, inLow.b, inHigh.b, outLow.b, outHigh.b, gamma.b, me->mIspc.doClamp);
        outValue.a = range(inValue.a, inLow.a, inHigh.a, outLow.a, outHigh.a, gamma.a, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = range(inValue.x, inLow.x, inHigh.x, outLow.x, outHigh.x, gamma.x, me->mIspc.doClamp);
        outValue.y = range(inValue.y, inLow.y, inHigh.y, outLow.y, outHigh.y, gamma.y, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = range(inValue.x, inLow.x, inHigh.x, outLow.x, outHigh.x, gamma.x, me->mIspc.doClamp);
        outValue.y = range(inValue.y, inLow.y, inHigh.y, outLow.y, outHigh.y, gamma.y, me->mIspc.doClamp);
        outValue.z = range(inValue.z, inLow.z, inHigh.z, outLow.z, outHigh.z, gamma.z, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = range(inValue.x, inLow.x, inHigh.x, outLow.x, outHigh.x, gamma.x, me->mIspc.doClamp);
        outValue.y = range(inValue.y, inLow.y, inHigh.y, outLow.y, outHigh.y, gamma.y, me->mIspc.doClamp);
        outValue.z = range(inValue.z, inLow.z, inHigh.z, outLow.z, outHigh.z, gamma.z, me->mIspc.doClamp);
        outValue.w = range(inValue.w, inLow.w, inHigh.w, outLow.w, outHigh.w, gamma.w, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = range(inValue.r, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.g = range(inValue.g, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.b = range(inValue.b, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = range(inValue.r, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.g = range(inValue.g, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.b = range(inValue.b, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.a = range(inValue.a, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = range(inValue.x, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.y = range(inValue.y, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = range(inValue.x, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.y = range(inValue.y, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.z = range(inValue.z, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = range(inValue.x, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.y = range(inValue.y, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.z = range(inValue.z, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
        outValue.w = range(inValue.w, inLow, inHigh, outLow, outHigh, gamma, me->mIspc.doClamp);
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

