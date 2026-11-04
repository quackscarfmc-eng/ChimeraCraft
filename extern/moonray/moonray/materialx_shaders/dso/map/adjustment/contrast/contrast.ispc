// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "contrast.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the folpivoting variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION contrast

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
// Handle pivot and amount contrasting types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA) || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define CONTRAST_TYPE Float
#define CONTRAST_ATTRKEY FloatAttrKeyISPC
#else
#define CONTRAST_TYPE IN_TYPE
#define CONTRAST_ATTRKEY IN_ATTRKEY
#endif

#define CONTRAST_EVAL CONCAT2(eval,CONTRAST_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types using Color
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
extern uniform CONTRAST_ATTRKEY* uniform pivotAttr;
extern uniform CONTRAST_ATTRKEY* uniform amountAttr;

varying float
contrast(varying float inValue,
         varying float pivot,
         varying float amount)
{
    // Early out: identity case
    if (isOne(amount)) return inValue;
    return (inValue - pivot) * max(0.0f, amount) + pivot;
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);
    const varying CONTRAST_TYPE pivot = CONTRAST_EVAL(map, tls, state, pivotAttr);
    const varying CONTRAST_TYPE amount = CONTRAST_EVAL(map, tls, state, amountAttr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in1 value unchanged
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = contrast(inValue, pivot, amount);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = contrast(inValue.r, pivot.r, amount.r);
        outValue.g = contrast(inValue.g, pivot.g, amount.g);
        outValue.b = contrast(inValue.b, pivot.b, amount.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = contrast(inValue.r, pivot.r, amount.r);
        outValue.g = contrast(inValue.g, pivot.g, amount.g);
        outValue.b = contrast(inValue.b, pivot.b, amount.b);
        outValue.a = contrast(inValue.a, pivot.a, amount.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = contrast(inValue.x, pivot.x, amount.x);
        outValue.y = contrast(inValue.y, pivot.y, amount.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = contrast(inValue.x, pivot.x, amount.x);
        outValue.y = contrast(inValue.y, pivot.y, amount.y);
        outValue.z = contrast(inValue.z, pivot.z, amount.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = contrast(inValue.x, pivot.x, amount.x);
        outValue.y = contrast(inValue.y, pivot.y, amount.y);
        outValue.z = contrast(inValue.z, pivot.z, amount.z);
        outValue.w = contrast(inValue.w, pivot.w, amount.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = contrast(inValue.r, pivot, amount);
        outValue.g = contrast(inValue.g, pivot, amount);
        outValue.b = contrast(inValue.b, pivot, amount);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = contrast(inValue.r, pivot, amount);
        outValue.g = contrast(inValue.g, pivot, amount);
        outValue.b = contrast(inValue.b, pivot, amount);
        outValue.a = contrast(inValue.a, pivot, amount);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = contrast(inValue.x, pivot, amount);
        outValue.y = contrast(inValue.y, pivot, amount);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = contrast(inValue.x, pivot, amount);
        outValue.y = contrast(inValue.y, pivot, amount);
        outValue.z = contrast(inValue.z, pivot, amount);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = contrast(inValue.x, pivot, amount);
        outValue.y = contrast(inValue.y, pivot, amount);
        outValue.z = contrast(inValue.z, pivot, amount);
        outValue.w = contrast(inValue.w, pivot, amount);
#endif
    }

    //
    // Do conversion to output type if necessary
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
