// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ramptb.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include "attributes.isph"

// supports the following variants.
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4
//

#define OPERATION ramptb
#define OUT_DEFAULT_FLOAT 0.0f

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC
#define OUT_DEFAULT OUT_DEFAULT_FLOAT

#elif STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define OUT_DEFAULT Vec2f_ctor(OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_DEFAULT Vec3f_ctor(OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_DEFAULT Vec4f_ctor(OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_DEFAULT Color_ctor(OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_DEFAULT Col4f_ctor(OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT, OUT_DEFAULT_FLOAT)

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif


struct SHADER_NAME {
    uniform bool disableMode;
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform valuebAttr;
extern uniform IN_ATTRKEY* uniform valuetAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE valueb = IN_EVAL(map, tls, state, valuebAttr);
    varying IN_TYPE valuet = IN_EVAL(map, tls, state, valuetAttr);

    const varying Vec2f texcoord = me->texcoordUseDefault ? state.mSt : evalTexcoordAttr(map, tls, state);
    const varying float v = texcoord.y;

    // Note: the v-coordinate requires an OIIO-related flip, v -> 1-v. 
    // Instead of evaluating 1-v, we simply swap the relevant args of the lerp() call below.

    varying IN_TYPE out = me->disableMode ? OUT_DEFAULT : lerp(valuet, valueb, v);

    //
    // convert to the output type if necessary
    //
#if STRING_CMP(IN_TYPE,Float)
    return Color_ctor(out, out, out);
#elif STRING_CMP(IN_TYPE,Vec2f)
    return Color_ctor(out.x, out.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    return Color_ctor(out.x, out.y, out.z);
#else
    return out;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
