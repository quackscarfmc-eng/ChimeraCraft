// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "conditional.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// Supports the following variants. These are the in1 and in2 types, followed by a letter to 
// specify the types of value 1 and value2. If there's no letter after the type, it assumes
// value1 and value2 have a float type. If there's a B, they are bool. If there's an I, they are int.
//      - color3
//      - color3B
//      - color3I
//      - color4
//      - color4B
//      - color4I
//      - float
//      - floatB
//      - floatI
//      - vector2
//      - vector2B
//      - vector2I
//      - vector3
//      - vector3B
//      - vector3I
//      - vector4
//      - vector4B
//      - vector4I
//
// It also supports the following operations:
//      - ifequal
//      - ifgreater
//      - ifgreatereq
// Use #define VARIANT and #define OPERATION before including this file

// OPERATIONS
// Since we want to use a different OPERATION value per file we need to
// define these here. This allows us to use the STRING_CMP macro.
// Usually, we would only need to define VARIANT values, which are already
// enumerated in MtlxVariantTypes.hh.
// (starting at 200 to avoid conflicting with MtlxVariantTypes)
#define UNIQUE_ID_ifequal               203
#define UNIQUE_ID_ifgreater             204
#define UNIQUE_ID_ifgreatereq           205

//
// Configure the variants
//
// First, handle the input type
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3B) || STRING_CMP(VARIANT,color3I)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4B) || STRING_CMP(VARIANT,color4I)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,floatB) || STRING_CMP(VARIANT,floatI)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2B) || STRING_CMP(VARIANT,vector2I)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3B) || STRING_CMP(VARIANT,vector3I)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4B) || STRING_CMP(VARIANT,vector4I)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Handle the value type
//
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector4)
#define VALUE_TYPE Float
#define VALUE_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,color3I) || STRING_CMP(VARIANT,color4I) || STRING_CMP(VARIANT,floatI) || STRING_CMP(VARIANT,vector2I) || STRING_CMP(VARIANT,vector3I) || STRING_CMP(VARIANT,vector4I)
#define VALUE_TYPE Int
#define VALUE_ATTRKEY IntAttrKeyISPC

#elif STRING_CMP(VARIANT,color3B) || STRING_CMP(VARIANT,color4B) || STRING_CMP(VARIANT,floatB) || STRING_CMP(VARIANT,vector2B) || STRING_CMP(VARIANT,vector3B) || STRING_CMP(VARIANT,vector4B)
#define VALUE_TYPE Bool
#define VALUE_ATTRKEY BoolAttrKeyISPC
#else
#error VARIANT is set to an invalid value
#endif

#define VALUE_EVAL CONCAT2(eval,VALUE_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with mutiple output types mapping to Color
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

extern uniform IN_ATTRKEY* uniform in1Attr;
extern uniform IN_ATTRKEY* uniform in2Attr;
extern uniform VALUE_ATTRKEY* uniform value1Attr;
extern uniform VALUE_ATTRKEY* uniform value2Attr;

static varying RETURN_TYPE 
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const varying IN_TYPE in1Value = IN_EVAL(map, tls, state, in1Attr);
    const varying IN_TYPE in2Value = IN_EVAL(map, tls, state, in2Attr);
    const varying VALUE_TYPE value1 = VALUE_EVAL(map, tls, state, value1Attr);
    const varying VALUE_TYPE value2 = VALUE_EVAL(map, tls, state, value2Attr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in1 value unchanged
        outValue = in1Value;
    } else {
#if STRING_CMP(OPERATION,ifequal)
        outValue = (value1 == value2) ? in1Value : in2Value;
#elif STRING_CMP(OPERATION,ifgreater)
        outValue = (value1 > value2) ? in1Value : in2Value;
#elif STRING_CMP(OPERATION,ifgreatereq)
        outValue = (value1 >= value2) ? in1Value : in2Value;
#elif defined(OPERATION)
#error OPERATION is set to an invalid value
#else
#error OPERATION isn't set
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
