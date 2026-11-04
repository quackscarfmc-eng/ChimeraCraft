// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file normalize.cc

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// use #define VARIANT before including this file
// vector2, vector3, vector4

#define OPERATION normalize

//
// Configure the variants
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
        outValue = inValue;
    } else {
        outValue = normalize(inValue);
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(IN_TYPE,Vec2f)
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
