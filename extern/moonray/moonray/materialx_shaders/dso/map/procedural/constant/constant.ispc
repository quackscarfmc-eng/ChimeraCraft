// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "constant.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following variants. 
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// boolean, integer, matrix33, matrix44
//

#define OPERATION constant

//
// Configure the variants
// The output type is always the same as VALUE_TYPE
//
#if STRING_CMP(VARIANT,float)
#define VALUE_TYPE Float
#define VALUE_ATTRKEY FloatAttrKeyISPC
#define VALUE_DEFAULT 0.0f

#elif STRING_CMP(VARIANT,vector2)
#define VALUE_TYPE Vec2f
#define VALUE_ATTRKEY Float2AttrKeyISPC
#define VALUE_DEFAULT Vec2f_ctor(0.0f,0.0f)

#elif STRING_CMP(VARIANT,vector3)
#define VALUE_TYPE Vec3f
#define VALUE_ATTRKEY Float3AttrKeyISPC
#define VALUE_DEFAULT Vec3f_ctor(0.0f,0.0f,0.0f)

#elif STRING_CMP(VARIANT,vector4)
#define VALUE_TYPE Vec4f
#define VALUE_ATTRKEY Float4AttrKeyISPC
#define VALUE_DEFAULT Vec4f_ctor(0.0f,0.0f,0.0f,0.0f)

#elif STRING_CMP(VARIANT,color3)
#define VALUE_TYPE Color
#define VALUE_ATTRKEY Float3AttrKeyISPC
#define VALUE_DEFAULT Col3f_ctor(0.0f,0.0f,0.0f)

#elif STRING_CMP(VARIANT,color4)
#define VALUE_TYPE Col4f
#define VALUE_ATTRKEY Float4AttrKeyISPC
#define VALUE_DEFAULT Col4f_ctor(0.0f,0.0f,0.0f,0.0f)

#elif STRING_CMP(VARIANT,matrix33) 
#define VALUE_TYPE Mat3f
#define VALUE_ATTRKEY Mat3fAttrKeyISPC
#define VALUE_DEFAULT Mat3f_ctor(1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f)

#elif STRING_CMP(VARIANT,matrix44) 
#define VALUE_TYPE Mat4f
#define VALUE_ATTRKEY Mat4fAttrKeyISPC
#define VALUE_DEFAULT Mat4f_ctor(1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f,1.0f)

#elif STRING_CMP(VARIANT,boolean)
#define VALUE_TYPE Int
#define VALUE_ATTRKEY IntAttrKeyISPC
#define VALUE_DEFAULT 0

#elif STRING_CMP(VARIANT,integer)
#define VALUE_TYPE Int
#define VALUE_ATTRKEY IntAttrKeyISPC
#define VALUE_DEFAULT 0


#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#if STRING_CMP(VALUE_TYPE,Bool)
#define VALUE_EVAL evalInt
#else
#define VALUE_EVAL CONCAT2(eval,VALUE_TYPE)
#endif

//
// Handle low and high clamping types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA) || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define CLAMP_TYPE Float
#define CLAMP_ATTRKEY FloatAttrKeyISPC
#else
#define CLAMP_TYPE VALUE_TYPE
#define CLAMP_ATTRKEY VALUE_ATTRKEY
#endif

#define CLAMP_EVAL CONCAT2(eval,CLAMP_TYPE)

//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(VALUE_TYPE,Float) || STRING_CMP(VALUE_TYPE,Vec2f) || STRING_CMP(VALUE_TYPE,Vec3f) || STRING_CMP(VALUE_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE VALUE_TYPE
#endif


//
// disableMode is a flag that gets initialized inValue the .cc update() function
// and TRUE indicates that the in1 value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform VALUE_ATTRKEY* uniform valueAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying VALUE_TYPE inValue;
    inValue = VALUE_EVAL(map, tls, state, valueAttr);

    varying VALUE_TYPE outValue = inValue;
    if (me->disableMode) {
        // when it's disabled then it just passes through the in1 value unchanged
        outValue = VALUE_DEFAULT;
    } 

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(VALUE_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(VALUE_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(VALUE_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
