// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "range.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the folinLowing variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION range

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT is not set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)


//
// Handle inLow and inHigh rangeing types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA) || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define RANGE_TYPE Float
#define RANGE_ATTRKEY FloatAttrKeyISPC
#else
#define RANGE_TYPE IN_TYPE
#define RANGE_ATTRKEY IN_ATTRKEY
#endif

#define RANGE_EVAL CONCAT2(eval,RANGE_TYPE)


#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif



//
// disableMode is a flag that gets initialized inValue the .cc update() function
// and TRUE indicates that the in1 value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
    uniform bool doClamp;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;
extern uniform RANGE_ATTRKEY* uniform inLowAttr;
extern uniform RANGE_ATTRKEY* uniform inHighAttr;
extern uniform RANGE_ATTRKEY* uniform outLowAttr;
extern uniform RANGE_ATTRKEY* uniform outHighAttr;
extern uniform RANGE_ATTRKEY* uniform gammaAttr;

varying float
range(varying float inValue,
      varying float inLow,
      varying float inHigh,
      varying float outLow,
      varying float outHigh,
      varying float gamma,
      uniform bool doClamp)
{
    // Avoid division by zero
    if (inHigh == inLow) {
        return outLow;
    }

    // Normalize input to 0–1 remap
    float t = (inValue - inLow) / (inHigh - inLow);

    // Apply gamma correction
    if (gamma != 1.0f && t > 0.0f) {
        t = pow(t, 1.0f / gamma);
    }

    // Clamp if requested
    if (doClamp) {
        t = clamp(t, 0.0f, 1.0f);
    }

    // Interpolate to output remap
    return outLow + t * (outHigh - outLow);
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);
    const varying RANGE_TYPE inLow = RANGE_EVAL(map, tls, state, inLowAttr);
    const varying RANGE_TYPE inHigh = RANGE_EVAL(map, tls, state, inHighAttr);
    const varying RANGE_TYPE outLow = RANGE_EVAL(map, tls, state, outLowAttr);
    const varying RANGE_TYPE outHigh = RANGE_EVAL(map, tls, state, outHighAttr);
    const varying RANGE_TYPE gamma = RANGE_EVAL(map, tls, state, gammaAttr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in1 value unchanged
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = range(inValue, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = range(inValue.r, inLow.r, inHigh.r, outLow.r, outHigh.r, gamma.r, me->doClamp);
        outValue.g = range(inValue.g, inLow.g, inHigh.g, outLow.g, outHigh.g, gamma.g, me->doClamp);
        outValue.b = range(inValue.b, inLow.b, inHigh.b, outLow.b, outHigh.b, gamma.b, me->doClamp);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = range(inValue.r, inLow.r, inHigh.r, outLow.r, outHigh.r, gamma.r, me->doClamp);
        outValue.g = range(inValue.g, inLow.g, inHigh.g, outLow.g, outHigh.g, gamma.g, me->doClamp);
        outValue.b = range(inValue.b, inLow.b, inHigh.b, outLow.b, outHigh.b, gamma.b, me->doClamp);
        outValue.a = range(inValue.a, inLow.a, inHigh.a, outLow.a, outHigh.a, gamma.a, me->doClamp);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = range(inValue.x, inLow.x, inHigh.x, outLow.x, outHigh.x, gamma.x, me->doClamp);
        outValue.y = range(inValue.y, inLow.y, inHigh.y, outLow.y, outHigh.y, gamma.y, me->doClamp);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = range(inValue.x, inLow.x, inHigh.x, outLow.x, outHigh.x, gamma.x, me->doClamp);
        outValue.y = range(inValue.y, inLow.y, inHigh.y, outLow.y, outHigh.y, gamma.y, me->doClamp);
        outValue.z = range(inValue.z, inLow.z, inHigh.z, outLow.z, outHigh.z, gamma.z, me->doClamp);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = range(inValue.x, inLow.x, inHigh.x, outLow.x, outHigh.x, gamma.x, me->doClamp);
        outValue.y = range(inValue.y, inLow.y, inHigh.y, outLow.y, outHigh.y, gamma.y, me->doClamp);
        outValue.z = range(inValue.z, inLow.z, inHigh.z, outLow.z, outHigh.z, gamma.z, me->doClamp);
        outValue.w = range(inValue.w, inLow.w, inHigh.w, outLow.w, outHigh.w, gamma.w, me->doClamp);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = range(inValue.r, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.g = range(inValue.g, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.b = range(inValue.b, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = range(inValue.r, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.g = range(inValue.g, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.b = range(inValue.b, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.a = range(inValue.a, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = range(inValue.x, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.y = range(inValue.y, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = range(inValue.x, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.y = range(inValue.y, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.z = range(inValue.z, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = range(inValue.x, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.y = range(inValue.y, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.z = range(inValue.z, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
        outValue.w = range(inValue.w, inLow, inHigh, outLow, outHigh, gamma, me->doClamp);
#endif
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(IN_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
