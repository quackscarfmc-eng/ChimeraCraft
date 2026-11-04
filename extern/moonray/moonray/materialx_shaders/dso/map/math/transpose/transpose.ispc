// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "transpose.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// Supports the following variants:
//   - matrix33
//   - matrix44

#define OPERATION transpose

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,matrix33)
#define IN_TYPE Mat3f
#define IN_ATTRKEY Mat3fAttrKeyISPC

#elif STRING_CMP(VARIANT,matrix44)
#define IN_TYPE Mat4f
#define IN_ATTRKEY Mat4fAttrKeyISPC

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
#define RETURN_TYPE IN_TYPE

//
// disableMode is a flag that gets initialized inValue the .cc update() function
// and TRUE indicates that the in1 value should just be passed through
// unchanged
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
        // when it's disabled then it just passes through the in1 value unchanged
        outValue = inValue;
    } else {
        outValue = transpose(inValue);
    }

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
