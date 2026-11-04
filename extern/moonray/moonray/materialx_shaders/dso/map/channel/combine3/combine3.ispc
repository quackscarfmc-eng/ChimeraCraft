// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "combine3.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, vector3

#define OPERATION combine3
#define IN_DEFAULT_FLOAT 0.0f

//
// Configure the variants
//
#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define EXTRACT_TYPE IN_TYPE
#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
#define RETURN_TYPE Color

//
// the disableMode flag is initialized in the .cc update() function
// and TRUE indicates that the inValue should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform in1Attr;
extern uniform IN_ATTRKEY* uniform in2Attr;
extern uniform IN_ATTRKEY* uniform in3Attr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying Float in1Value;
    in1Value = evalFloat(map, tls, state, in1Attr);

    varying Float in2Value;
    in2Value = evalFloat(map, tls, state, in2Attr);

    varying Float in3Value;
    in3Value = evalFloat(map, tls, state, in3Attr);

    varying RETURN_TYPE outValue; // Should always be Color
    if (me->disableMode) {
        outValue.r = 0.f;
        outValue.g = 0.f;
        outValue.b = 0.f;
    } else {
        outValue.r = in1Value;
        outValue.g = in2Value;
        outValue.b = in3Value;
    }

    return outValue;
}

#define DEFINE_MAP_EVALUATOR(shadername, funcname) DEFINE_MAP_SHADER(shadername, funcname)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample)
