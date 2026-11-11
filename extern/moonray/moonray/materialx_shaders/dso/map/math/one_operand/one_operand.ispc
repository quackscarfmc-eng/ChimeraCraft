// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "one_operand.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following OPERATIONS with one input
// acos, asin, ceil, cos, exp, floor, ln, round, sign, sin, sqrt, tan
//
// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, matrix33, matrix44
// color3FA, vector2FA, vector3FA, vector4FA, matrix33FA, matrix44FA
//
// Not every combination of OPERATION and VARIANT is a standard MaterialX shader
// See README for list of shaders.

// OPERATIONS
// must define these for our STRING_CMP macro to work
#define UNIQUE_ID_absval 200
#define UNIQUE_ID_floor  201
#define UNIQUE_ID_ceil   202
#define UNIQUE_ID_round  203
#define UNIQUE_ID_sin    204
#define UNIQUE_ID_cos    205
#define UNIQUE_ID_tan    206
#define UNIQUE_ID_asin   207
#define UNIQUE_ID_acos   208
#define UNIQUE_ID_sqrt   209
#define UNIQUE_ID_ln     210
#define UNIQUE_ID_sign   211
#define UNIQUE_ID_exp    212

//
// Configure the variants
// The output type is always the same as IN
//
#if STRING_CMP(VARIANT,integer)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,matrix33)
#define IN_TYPE Mat3f
#define IN_ATTRKEY Mat3fAttrKeyISPC

#elif STRING_CMP(VARIANT,matrix44)
#define IN_TYPE Mat4f
#define IN_ATTRKEY Mat4fAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2FA)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3FA)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4FA)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,color3FA)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4FA)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,matrix33FA)
#define IN_TYPE Mat3f
#define IN_ATTRKEY Mat3fAttrKeyISPC

#elif STRING_CMP(VARIANT,matrix44FA)
#define IN_TYPE Mat4f
#define IN_ATTRKEY Mat4fAttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,color3)
#define RETURN_TYPE Color
#else
#if STRING_CMP(VARIANT,integer)
#define RETURN_TYPE Int
#else
#define RETURN_TYPE IN_TYPE
#endif
#endif

//
// disableMode is a flag that gets initialized in the .cc update() function
// and TRUE indicates that the in value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

static varying
float doOperation(varying float& in1)
{
#if STRING_CMP(OPERATION,absval)
    if (in1 < 0.0) {
        return -in1;
    } else {
        return in1;
    }
#elif STRING_CMP(OPERATION,floor)
    return floor(in1);
#elif STRING_CMP(OPERATION,ceil)
    return ceil(in1);
#elif STRING_CMP(OPERATION,round)
    return round(in1);
#elif STRING_CMP(OPERATION,sin)
    return sin(in1);
#elif STRING_CMP(OPERATION,cos)
    return cos(in1);
#elif STRING_CMP(OPERATION,tan)
    return tan(in1);
#elif STRING_CMP(OPERATION,asin)
    return asin(in1);
#elif STRING_CMP(OPERATION,acos)
    return acos(in1);
#elif STRING_CMP(OPERATION,sqrt)
    return sqrt(in1);
#elif STRING_CMP(OPERATION,ln)
    return log(in1);
#elif STRING_CMP(OPERATION,exp)
    return exp(in1);
#elif STRING_CMP(OPERATION,sign)
    if (in1 > 0.0) {
        return 1.0;
    } else if (in1 < 0.0) {
        return -1.0;
    } else {
        return 0.0;
    }
#elif defined(OPERATION)
    #error OPERATION is set to an invalid value
#else
    #error OPERATION isn't set
#endif
}

extern uniform IN_ATTRKEY* uniform inAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in value unchanged
        outValue = inValue;
    } else {

//
// doOperation is just a single float but the compiler should be able
// optimize out all of the function calls.
//
#if STRING_CMP(VARIANT,integer)
        outValue = doOperation(inValue);
#elif STRING_CMP(VARIANT,float)
        outValue = doOperation(inValue);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = doOperation(inValue.r);
        outValue.g = doOperation(inValue.g);
        outValue.b = doOperation(inValue.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = doOperation(inValue.r);
        outValue.g = doOperation(inValue.g);
        outValue.b = doOperation(inValue.b);
        outValue.a = doOperation(inValue.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = doOperation(inValue.x);
        outValue.y = doOperation(inValue.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = doOperation(inValue.x);
        outValue.y = doOperation(inValue.y);
        outValue.z = doOperation(inValue.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = doOperation(inValue.x);
        outValue.y = doOperation(inValue.y);
        outValue.z = doOperation(inValue.z);
        outValue.w = doOperation(inValue.w);
#endif
    }

    //
    // Convert to the output type if necessary
    //
    varying RETURN_TYPE result;
#if STRING_CMP(IN_TYPE,Float) && STRING_CMP(VARIANT,float)
    result = Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    result = Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    result = Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    result = outValue;
#endif

    return result;
}

// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
