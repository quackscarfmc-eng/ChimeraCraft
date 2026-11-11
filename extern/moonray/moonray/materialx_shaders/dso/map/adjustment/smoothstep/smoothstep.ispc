// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "smoothstep.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the folinLowing variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION smoothstep

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
// Handle inLow and inHigh smoothsteping types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA) || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define SMOOTHSTEP_TYPE Float
#define SMOOTHSTEP_ATTRKEY FloatAttrKeyISPC
#else
#define SMOOTHSTEP_TYPE IN_TYPE
#define SMOOTHSTEP_ATTRKEY IN_ATTRKEY
#endif

#define SMOOTHSTEP_EVAL CONCAT2(eval,SMOOTHSTEP_TYPE)


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
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;
extern uniform SMOOTHSTEP_ATTRKEY* uniform lowAttr;
extern uniform SMOOTHSTEP_ATTRKEY* uniform highAttr;

varying float
smoothstep(varying float inValue,
           varying float low,
           varying float high)
{
    // Avoid divide-by-zero
    if (low == high) return 0.0f;

    // Detect direction
    bool inverted = (low > high);
    if (inverted) {
        float tmp = low;
        low = high;
        high = tmp;
    }

    float t = (inValue - low) / (high - low);
    t = clamp(t, 0.0f, 1.0f);
    float result = t * t * (3.0f - 2.0f * t);

    return inverted ? 1.0f - result : result;
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);
    const varying SMOOTHSTEP_TYPE low = SMOOTHSTEP_EVAL(map, tls, state, lowAttr);
    const varying SMOOTHSTEP_TYPE high = SMOOTHSTEP_EVAL(map, tls, state, highAttr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in1 value unchanged
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
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    // all of the Map output types can just return the value
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
