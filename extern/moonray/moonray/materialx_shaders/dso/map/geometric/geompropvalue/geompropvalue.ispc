// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "geompropvalue.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>

#define OPERATION geompropvalue

//
// Configure the variants
// The output type is always the same as DEFAULT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define DEFAULT_TYPE Float
#define DEFAULT_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,boolean)
#define DEFAULT_TYPE Bool
#define DEFAULT_ATTRKEY BoolAttrKeyISPC

#elif STRING_CMP(VARIANT,integer)
#define DEFAULT_TYPE Int
#define DEFAULT_ATTRKEY IntAttrKeyISPC

#elif STRING_CMP(VARIANT,string)
#define DEFAULT_TYPE String
#define DEFAULT_ATTRKEY StringAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2)
#define DEFAULT_TYPE Vec2f
#define DEFAULT_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)
#define DEFAULT_TYPE Vec3f
#define DEFAULT_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4)
#define DEFAULT_TYPE Vec4f
#define DEFAULT_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,color3)
#define DEFAULT_TYPE Color
#define DEFAULT_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4)
#define DEFAULT_TYPE Col4f
#define DEFAULT_ATTRKEY Float4AttrKeyISPC

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
#if STRING_CMP(DEFAULT_TYPE,Float) || STRING_CMP(DEFAULT_TYPE,Vec2f) || STRING_CMP(DEFAULT_TYPE,Vec3f) || STRING_CMP(DEFAULT_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE DEFAULT_TYPE
#endif



//
// mDisableMode is a flag that gets initialized in the .cc update() function
// and TRUE indicates that the default value should returned
// unchanged
struct SHADER_NAME {
    uniform bool mDisableMode;
    uniform int mPrimitiveAttributeIndex;
    uniform int mMissingAttributeEvent;
    uniform int mUnsupportedStringEvent;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform DEFAULT_ATTRKEY* uniform defaultAttr;


static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&          state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const int key = me->mPrimitiveAttributeIndex;

    DEFAULT_TYPE outValue;

    if (me->mDisableMode) {
        outValue = DEFAULT_EVAL(map, tls, state, defaultAttr);
    } else {
        if (isProvided(state, key)) {
#if STRING_CMP(VARIANT,float)
            outValue = getFloatAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,color3)
            outValue = getColorAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,color4)
            outValue = getCol4fAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,vector2)
            outValue = getVec2fAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,vector3)
            outValue = getVec3fAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,vector4)
            outValue = getVec4fAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,boolean)
            outValue = getBoolAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,integer)
            outValue = getIntAttribute(tls, state, key);
#elif STRING_CMP(VARIANT,string)
            logEvent((const uniform Map *)map, me->mUnsupportedStringEvent);
            outValue = DEFAULT_EVAL(map, tls, state, defaultAttr);
#endif
        } else {
            logEvent((const uniform Map * uniform)map, me->mMissingAttributeEvent);
            outValue = DEFAULT_EVAL(map, tls, state, defaultAttr);
        }
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(DEFAULT_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(DEFAULT_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(DEFAULT_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
