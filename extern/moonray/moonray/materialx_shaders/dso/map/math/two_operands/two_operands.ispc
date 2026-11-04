// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "two_operands.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following componentwise operations with two float arguments
// use #define OPERATION before including this file
// add, subtract, multiply, divide, modulo, atan2, power, safepower, min, max
//
// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, matrix33, matrix44
// color3FA, vector2FA, vector3FA, vector4FA, matrix33FA, matrix44FA
//
// Not every combination of OPERATION and VARIANT is a standard MaterialX shader
// See README for list of shaders.

// OPERATIONS
#define UNIQUE_ID_add       200
#define UNIQUE_ID_subtract  201
#define UNIQUE_ID_multiply  202
#define UNIQUE_ID_divide    203
#define UNIQUE_ID_modulo    204
#define UNIQUE_ID_atan2     205
#define UNIQUE_ID_power     206
#define UNIQUE_ID_safepower 207
#define UNIQUE_ID_min       208
#define UNIQUE_ID_max       209

//
// Configure the variants
// The output type is always the same as IN1
//
#if STRING_CMP(VARIANT,float)

#define IN1_TYPE Float
#define IN1_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)

#define IN1_TYPE Vec2f
#define IN1_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)

#define IN1_TYPE Vec3f
#define IN1_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)

#define IN1_TYPE Vec4f
#define IN1_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)

#define IN1_TYPE Color
#define IN1_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)

#define IN1_TYPE Col4f
#define IN1_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,matrix33) || STRING_CMP(VARIANT,matrix33FA)

#define IN1_TYPE Mat3f
#define IN1_ATTRKEY Mat3fAttrKeyISPC

#elif STRING_CMP(VARIANT,matrix44) || STRING_CMP(VARIANT,matrix44FA)

#define IN1_TYPE Mat4f
#define IN1_ATTRKEY Mat4fAttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA) || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA) || STRING_CMP(VARIANT,matrix33FA) || STRING_CMP(VARIANT,matrix44FA)
#define IN2_TYPE Float
#define IN2_ATTRKEY FloatAttrKeyISPC
#else
#define IN2_TYPE IN1_TYPE
#define IN2_ATTRKEY IN1_ATTRKEY
#endif

#define IN1_EVAL CONCAT2(eval,IN1_TYPE)
#define IN2_EVAL CONCAT2(eval,IN2_TYPE)

//
// Deal with with multiple output types mapping to Color
//
#if STRING_CMP(IN1_TYPE,Float) || STRING_CMP(IN1_TYPE,Vec2f) || STRING_CMP(IN1_TYPE,Vec3f) || STRING_CMP(IN1_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN1_TYPE
#endif

//
// disableMode is a flag that gets initialized in the .cc update() function
// and TRUE indicates that the in1 value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

static varying
float doOperation(varying float& in1, varying float& in2)
{
#if STRING_CMP(OPERATION,add)
    return in1 + in2;
#elif STRING_CMP(OPERATION,subtract)
    return in1 - in2;
#elif STRING_CMP(OPERATION,multiply)
    return in1 * in2;
#elif STRING_CMP(OPERATION,divide)
    return in1 / in2;
#elif STRING_CMP(OPERATION,modulo)
    return fmod(in1, in2);
#elif STRING_CMP(OPERATION,atan2)
    return atan2(in1, in2);
#elif STRING_CMP(OPERATION,power)
    return pow(in1, in2);
#elif STRING_CMP(OPERATION,safepower)
    // prevents trying to take roots of negative numbers
    return (in1 < 0.0) ? -pow(-in1,in2) : pow(in1,in2);
#elif STRING_CMP(OPERATION,min)
    return min(in1, in2);
#elif STRING_CMP(OPERATION,max)
    return max(in1, in2);
#elif defined(OPERATION)
    #error OPERATION is set to an invalid value
#else
    #error OPERATION isn't set
#endif
}

extern uniform IN1_ATTRKEY* uniform in1Attr;
extern uniform IN2_ATTRKEY* uniform in2Attr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN1_TYPE in1Value;
    in1Value = IN1_EVAL(map, tls, state, in1Attr);

    varying IN2_TYPE in2Value;
    in2Value = IN2_EVAL(map, tls, state, in2Attr);

    varying IN1_TYPE outValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in1 value unchanged
        outValue = in1Value;
    } else {

//
// doOperation is just a single float but the compiler should be able
// optimize out all of the function calls.
//
#if STRING_CMP(VARIANT,float)
        outValue = doOperation(in1Value, in2Value);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = doOperation(in1Value.r, in2Value.r);
        outValue.g = doOperation(in1Value.g, in2Value.g);
        outValue.b = doOperation(in1Value.b, in2Value.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = doOperation(in1Value.r, in2Value.r);
        outValue.g = doOperation(in1Value.g, in2Value.g);
        outValue.b = doOperation(in1Value.b, in2Value.b);
        outValue.a = doOperation(in1Value.a, in2Value.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = doOperation(in1Value.x, in2Value.x);
        outValue.y = doOperation(in1Value.y, in2Value.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = doOperation(in1Value.x, in2Value.x);
        outValue.y = doOperation(in1Value.y, in2Value.y);
        outValue.z = doOperation(in1Value.z, in2Value.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = doOperation(in1Value.x, in2Value.x);
        outValue.y = doOperation(in1Value.y, in2Value.y);
        outValue.z = doOperation(in1Value.z, in2Value.z);
        outValue.w = doOperation(in1Value.w, in2Value.w);

//
// matrix multiply and divide are special cases defined in the spec,
// not component-wise operations
//
#elif STRING_CMP(OPERATION,multiply) && (STRING_CMP(VARIANT,matrix33) || STRING_CMP(VARIANT,matrix44))
        outValue = in1Value * in2Value;
#elif STRING_CMP(OPERATION,divide) && (STRING_CMP(VARIANT,matrix33) || STRING_CMP(VARIANT,matrix44))
        outValue = in1Value * inverse(in2Value);

#elif STRING_CMP(VARIANT,matrix33)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value.vx.x);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value.vx.y);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value.vx.z);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value.vy.x);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value.vy.y);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value.vy.z);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value.vz.x);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value.vz.y);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value.vz.z);
#elif STRING_CMP(VARIANT,matrix44)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value.vx.x);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value.vx.y);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value.vx.z);
        outValue.vx.w = doOperation(in1Value.vx.w, in2Value.vx.w);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value.vy.x);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value.vy.y);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value.vy.z);
        outValue.vy.w = doOperation(in1Value.vy.w, in2Value.vy.w);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value.vz.x);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value.vz.y);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value.vz.z);
        outValue.vz.w = doOperation(in1Value.vz.w, in2Value.vz.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = doOperation(in1Value.r, in2Value);
        outValue.g = doOperation(in1Value.g, in2Value);
        outValue.b = doOperation(in1Value.b, in2Value);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = doOperation(in1Value.r, in2Value);
        outValue.g = doOperation(in1Value.g, in2Value);
        outValue.b = doOperation(in1Value.b, in2Value);
        outValue.a = doOperation(in1Value.a, in2Value);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = doOperation(in1Value.x, in2Value);
        outValue.y = doOperation(in1Value.y, in2Value);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = doOperation(in1Value.x, in2Value);
        outValue.y = doOperation(in1Value.y, in2Value);
        outValue.z = doOperation(in1Value.z, in2Value);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = doOperation(in1Value.x, in2Value);
        outValue.y = doOperation(in1Value.y, in2Value);
        outValue.z = doOperation(in1Value.z, in2Value);
        outValue.w = doOperation(in1Value.w, in2Value);
#elif STRING_CMP(VARIANT,matrix33FA)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value);
#elif STRING_CMP(VARIANT,matrix44FA)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value);
        outValue.vx.w = doOperation(in1Value.vx.w, in2Value);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value);
        outValue.vy.w = doOperation(in1Value.vy.w, in2Value);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value);
        outValue.vz.w = doOperation(in1Value.vz.w, in2Value);
#endif
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(IN1_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(IN1_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN1_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
