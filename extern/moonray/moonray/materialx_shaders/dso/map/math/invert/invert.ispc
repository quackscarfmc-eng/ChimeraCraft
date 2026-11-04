// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "invert.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION invert

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
#error VARIANT isn't set
#endif

//
// Set up the amountAttr type
//
#if STRING_CMP(VARIANT, float) || STRING_CMP(VARIANT, vector2FA) || STRING_CMP(VARIANT, vector3FA) || \
    STRING_CMP(VARIANT, vector4FA) || STRING_CMP(VARIANT, color3FA) || STRING_CMP(VARIANT, color4FA)
#define AMOUNT_TYPE Float
#define AMOUNT_ATTRKEY FloatAttrKeyISPC
#else
#define AMOUNT_TYPE IN_TYPE
#define AMOUNT_ATTRKEY IN_ATTRKEY
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)
#define AMOUNT_EVAL CONCAT2(eval,AMOUNT_TYPE)

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
extern uniform AMOUNT_ATTRKEY* uniform amountAttr;

varying float
invert(varying float inValue, varying float amount)
{
    return amount - inValue;
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    varying AMOUNT_TYPE amountValue;
    inValue = IN_EVAL(map, tls, state, inAttr);
    amountValue = AMOUNT_EVAL(map, tls, state, amountAttr);
    
    varying IN_TYPE outValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in1 value unchanged
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = invert(inValue, amountValue);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = invert(inValue.r, amountValue.r);
        outValue.g = invert(inValue.g, amountValue.g);
        outValue.b = invert(inValue.b, amountValue.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = invert(inValue.r, amountValue.r);
        outValue.g = invert(inValue.g, amountValue.g);
        outValue.b = invert(inValue.b, amountValue.b);
        outValue.a = invert(inValue.a, amountValue.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = invert(inValue.x, amountValue.x);
        outValue.y = invert(inValue.y, amountValue.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = invert(inValue.x, amountValue.x);
        outValue.y = invert(inValue.y, amountValue.y);
        outValue.z = invert(inValue.z, amountValue.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = invert(inValue.x, amountValue.x);
        outValue.y = invert(inValue.y, amountValue.y);
        outValue.z = invert(inValue.z, amountValue.z);
        outValue.w = invert(inValue.w, amountValue.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = invert(inValue.r, amountValue);
        outValue.g = invert(inValue.g, amountValue);
        outValue.b = invert(inValue.b, amountValue);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = invert(inValue.r, amountValue);
        outValue.g = invert(inValue.g, amountValue);
        outValue.b = invert(inValue.b, amountValue);
        outValue.a = invert(inValue.a, amountValue);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = invert(inValue.x, amountValue);
        outValue.y = invert(inValue.y, amountValue);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = invert(inValue.x, amountValue);
        outValue.y = invert(inValue.y, amountValue);
        outValue.z = invert(inValue.z, amountValue);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = invert(inValue.x, amountValue);
        outValue.y = invert(inValue.y, amountValue);
        outValue.z = invert(inValue.z, amountValue);
        outValue.w = invert(inValue.w, amountValue);
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
