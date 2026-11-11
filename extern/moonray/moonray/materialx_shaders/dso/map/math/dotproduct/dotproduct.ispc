// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "dotproduct.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// Supports the following variants:
//      - vector2
//      - vector3
//      - vector4
// Use #define VARIANT before including this file

#define OPERATION dotproduct

//
// Configure the variants
// The output type is always the same as IN_TYPE
//

#if STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Map shader
//
#include <moonray/rendering/shading/ispc/MapApi.isph>

// disableMode is a flag that gets initialized inValue the .cc update() function
// and TRUE indicates that the default output value (0.0) should be output
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform in1Attr;
extern uniform IN_ATTRKEY* uniform in2Attr;

static varying Color
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE in1Value;
    varying IN_TYPE in2Value;
    in1Value = IN_EVAL(map, tls, state, in1Attr);
    in2Value = IN_EVAL(map, tls, state, in2Attr);

    varying float outValue;
    if (me->disableMode) {
        outValue = 0.0;
    } else {
#if STRING_CMP(VARIANT,vector2)
        outValue = in1Value.x * in2Value.x + in1Value.y * in2Value.y;
#elif STRING_CMP(VARIANT,vector3)
        outValue = in1Value.x * in2Value.x + in1Value.y * in2Value.y + in1Value.z * in2Value.z;
#elif STRING_CMP(VARIANT,vector4)
        outValue = in1Value.x * in2Value.x + in1Value.y * in2Value.y + in1Value.z * in2Value.z + in1Value.w * in2Value.w;
#endif
    }

    return Color_ctor(outValue, outValue, outValue);
}

// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname) DEFINE_MAP_SHADER(shadername, funcname)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample)
