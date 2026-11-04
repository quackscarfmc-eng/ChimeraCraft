// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file magnitude.cc

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// use #define VARIANT before including this file
// vector2, vector3, vector4

#define OPERATION magnitude

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

#include <moonray/rendering/shading/ispc/MapApi.isph>
#define RETURN_TYPE Color

struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);

    varying Color outValue;
    if (me->disableMode) {
#if STRING_CMP(VARIANT,vector2)
        outValue = Color_ctor(inValue.x, inValue.y, 0.0f);
#elif STRING_CMP(VARIANT,vector3)
        outValue = Color_ctor(inValue.x, inValue.y, inValue.z);
#else
        // We don't support vector4 outputs yet, so we'll just drop the last
        // component for now.
        outValue = Color_ctor(inValue.x, inValue.y, inValue.z);
#endif
    } else {
        const varying float magnitude = length(inValue);
        outValue = Color_ctor(magnitude);
    }

    return outValue;
}

#define DEFINE_MAP_EVALUATOR(shadername, funcname) DEFINE_MAP_SHADER(shadername, funcname)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample)

