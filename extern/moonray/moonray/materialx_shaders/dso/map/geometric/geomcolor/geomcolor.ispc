// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "geomcolor.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>

#define OPERATION geomcolor

//
// Configure the variants
// The output type is always the same as DEFAULT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define DEFAULT_TYPE Float

#elif STRING_CMP(VARIANT,color3)
#define DEFAULT_TYPE Color

#elif STRING_CMP(VARIANT,color4)
#define DEFAULT_TYPE Col4f

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT is not set
#endif

#define DEFAULT_EVAL CONCAT2(eval,DEFAULT_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(DEFAULT_TYPE,Float) || STRING_CMP(DEFAULT_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE DEFAULT_TYPE
#endif



struct SHADER_NAME {
    uniform int mPrimitiveAttributeIndex;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IntAttrKeyISPC* uniform indexAttr;


static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&          state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const int key = me->mPrimitiveAttributeIndex;

    DEFAULT_TYPE outValue;
    
    if (isProvided(state, key)) {
#if STRING_CMP(VARIANT,float)
        outValue = getFloatAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,color3)
        outValue = getColorAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,color4)
        outValue = getCol4fAttribute(tls, state, key);
#endif
    } else {
        // Use default values when attribute is not provided
#if STRING_CMP(VARIANT,float)
        outValue = 0.0f;
#elif STRING_CMP(VARIANT,color3)
        outValue = Color_ctor(0.0f, 0.0f, 0.0f);
#elif STRING_CMP(VARIANT,color4)
        outValue = Col4f_ctor(0.0f, 0.0f, 0.0f, 1.0f);
#endif
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(DEFAULT_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#else
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
