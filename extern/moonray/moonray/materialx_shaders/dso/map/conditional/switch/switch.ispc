// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_switch_color3I.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// Supports the following variants. These are the in1 - in5 types, followed by a letter to 
// specify the type of "which". If there's no letter after the type, it assumes
// "which" has a float type. If there's an I, it's an int.
//      - color3
//      - color3I
//      - color4
//      - color4I
//      - float
//      - floatI
//      - vector2
//      - vector2I
//      - vector3
//      - vector3I
//      - vector4
//      - vector4I
//
// Use #define VARIANT before including this file

#define OPERATION switch

//
// Configure the variants
//
// First, handle the input type
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3I)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4I)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,floatI)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2I)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3I)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4I)
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
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,float) || \
    STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector4)
#define WHICH_TYPE Float
#define WHICH_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,color3I) || STRING_CMP(VARIANT,color4I) || STRING_CMP(VARIANT,floatI) || \
      STRING_CMP(VARIANT,vector2I) || STRING_CMP(VARIANT,vector3I) || STRING_CMP(VARIANT,vector4I)
#define WHICH_TYPE Int
#define WHICH_ATTRKEY IntAttrKeyISPC

#else
#error VARIANT is set to an invalid value
#endif

#define WHICH_EVAL CONCAT2(eval,WHICH_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif

struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform in1Attr;
extern uniform IN_ATTRKEY* uniform in2Attr;
extern uniform IN_ATTRKEY* uniform in3Attr;
extern uniform IN_ATTRKEY* uniform in4Attr;
extern uniform IN_ATTRKEY* uniform in5Attr;
extern uniform WHICH_ATTRKEY* uniform whichAttr;

static RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE in1Value = IN_EVAL(map, tls, state, in1Attr);
    varying IN_TYPE in2Value = IN_EVAL(map, tls, state, in2Attr);
    varying IN_TYPE in3Value = IN_EVAL(map, tls, state, in3Attr);
    varying IN_TYPE in4Value = IN_EVAL(map, tls, state, in4Attr);
    varying IN_TYPE in5Value = IN_EVAL(map, tls, state, in5Attr);
    varying WHICH_TYPE whichValue = WHICH_EVAL(map, tls, state, whichAttr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        outValue = in1Value;
    } else {
        switch ((int)whichValue) {
            case 0: outValue = in1Value; break;
            case 1: outValue = in2Value; break;
            case 2: outValue = in3Value; break;
            case 3: outValue = in4Value; break;
            case 4: outValue = in5Value; break;
            default: outValue = in1Value; break;
        }
    }
    //
    // Convert to the output type if necessary.
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
