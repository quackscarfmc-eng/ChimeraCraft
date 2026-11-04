// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "extract.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4, vector2, vector3, vector4

#define OPERATION extract
#define IN_DEFAULT_FLOAT 0.0f

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,vector2)
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

#define EXTRACT_TYPE IN_TYPE
#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>

//
// the disableMode flag is initialized in the .cc update() function
// and TRUE indicates that the inValue should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;
extern uniform IntAttrKeyISPC* uniform indexAttr;

varying float
extractChannel(const varying IN_TYPE& val, int index)
{
    #if STRING_CMP(VARIANT,color3)
        switch (index) {
            case 0: return val.r;
            case 1: return val.g;
            default: return val.b;
        }
    #elif STRING_CMP(VARIANT,color4)
        switch (index) {
            case 0: return val.r;
            case 1: return val.g;
            case 2: return val.b;
            default: return val.a;
        }
    #elif STRING_CMP(VARIANT,vector2)
        switch (index) {
            case 0: return val.x;
            default: return val.y;
        }
    #elif STRING_CMP(VARIANT,vector3)
        switch (index) {
            case 0: return val.x;
            case 1: return val.y;
            default: return val.z;
        }
    #elif STRING_CMP(VARIANT,vector4)
        switch (index) {
            case 0: return val.x;
            case 1: return val.y;
            case 2: return val.z;
            default: return val.w;
        }
    #endif
}

static varying Color
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);

    varying int indexValue;
    indexValue = evalInt(map, tls, state, indexAttr);

    varying Color outValue;
    if (me->disableMode) {
        outValue = Color_ctor(0.f, 0.f, 0.f);
    } else {
        float v = extractChannel(inValue, indexValue);
        outValue.r = v;
        outValue.g = v;
        outValue.b = v;
    }

    return outValue;
}

// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument

#define DEFINE_MAP_EVALUATOR(shadername, funcname) DEFINE_MAP_SHADER(shadername, funcname)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample)
