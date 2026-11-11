// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file inside.ispc

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>

#define OPERATION inside

#if STRING_CMP(VARIANT,float)

#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,color3)

#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4)

#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif


#define IN_EVAL CONCAT2(eval,IN_TYPE)

#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif


//
// disableMode is a flag that gets initialized in the .cc update() function
// and TRUE indicates that the bg value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);


extern uniform IN_ATTRKEY* uniform inAttr;
extern uniform FloatAttrKeyISPC* uniform maskAttr;


static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const varying IN_TYPE inValue = IN_EVAL(map, tls, state, inAttr);
    const varying float maskValue = evalFloat(map, tls, state, maskAttr);

    const varying IN_TYPE outValue = me->disableMode ? inValue : (1.0 - maskValue) * inValue;

#if STRING_CMP(IN_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#else
    return outValue;
#endif
}

// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
