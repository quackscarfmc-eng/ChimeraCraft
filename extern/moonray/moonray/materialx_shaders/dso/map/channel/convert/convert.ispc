// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "convert.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// only support OPERATION "convert"
#define OPERATION convert

//
// Configure the variants
//
#if STRING_CMP(VARIANT,float_color3)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC
#define OUT_TYPE Color

#elif STRING_CMP(VARIANT,float_color4)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC
#define OUT_TYPE Col4f

#elif STRING_CMP(VARIANT,float_vector2)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC
#define OUT_TYPE Vec2f

#elif STRING_CMP(VARIANT,float_vector3)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC
#define OUT_TYPE Vec3f

#elif STRING_CMP(VARIANT,float_vector4)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC
#define OUT_TYPE Vec4f

#elif STRING_CMP(VARIANT,color3_color4)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Col4f

#elif STRING_CMP(VARIANT,color3_vector2)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Vec2f

#elif STRING_CMP(VARIANT,color3_vector3)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Vec3f

#elif STRING_CMP(VARIANT,color3_vector4)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Vec4f

#elif STRING_CMP(VARIANT,color4_color3)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Color

#elif STRING_CMP(VARIANT,color4_vector2)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Vec2f

#elif STRING_CMP(VARIANT,color4_vector3)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Vec3f

#elif STRING_CMP(VARIANT,color4_vector4)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Vec4f

#elif STRING_CMP(VARIANT,vector2_color3)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define OUT_TYPE Color

#elif STRING_CMP(VARIANT,vector2_color4)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define OUT_TYPE Col4f

#elif STRING_CMP(VARIANT,vector2_vector3)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define OUT_TYPE Vec3f

#elif STRING_CMP(VARIANT,vector2_vector4)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define OUT_TYPE Vec4f

#elif STRING_CMP(VARIANT,vector3_color3)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Color

#elif STRING_CMP(VARIANT,vector3_color4)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Col4f

#elif STRING_CMP(VARIANT,vector3_vector2)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Vec2f

#elif STRING_CMP(VARIANT,vector3_vector4)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_TYPE Vec4f

#elif STRING_CMP(VARIANT,vector4_color3)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Color

#elif STRING_CMP(VARIANT,vector4_color4)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Col4f

#elif STRING_CMP(VARIANT,vector4_vector2)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Vec2f

#elif STRING_CMP(VARIANT,vector4_vector3)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_TYPE Vec3f

#elif STRING_CMP(VARIANT,boolean_float)
#define IN_TYPE Bool
#define IN_ATTRKEY BoolAttrKeyISPC
#define OUT_TYPE Float

#elif STRING_CMP(VARIANT,integer_float)
#define IN_TYPE Int
#define IN_ATTRKEY IntAttrKeyISPC
#define OUT_TYPE Float

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT is not set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(OUT_TYPE,Float) || STRING_CMP(OUT_TYPE,Vec2f) || STRING_CMP(OUT_TYPE,Vec3f) || STRING_CMP(OUT_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE OUT_TYPE
#endif

extern uniform IN_ATTRKEY* uniform inAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);

    varying OUT_TYPE outValue;
#if STRING_CMP(VARIANT,float_color3)
    outValue = Color_ctor(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,float_color4)
    outValue = Col4f_ctor(inValue, inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,float_vector2)
    outValue = Vec2f_ctor(inValue, inValue);
#elif STRING_CMP(VARIANT,float_vector3)
    outValue = Vec3f_ctor(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,float_vector4)
    outValue = Vec4f_ctor(inValue, inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,color3_color4)
    outValue = Col4f_ctor(inValue.r, inValue.g, inValue.b, 1.0);
#elif STRING_CMP(VARIANT,color3_vector2)
    outValue = Vec2f_ctor(inValue.r, inValue.g);
#elif STRING_CMP(VARIANT,color3_vector3)
    outValue = Vec3f_ctor(inValue.r, inValue.g, inValue.b);
#elif STRING_CMP(VARIANT,color3_vector4)
    outValue = Vec4f_ctor(inValue.r, inValue.g, inValue.b, 1.0);
#elif STRING_CMP(VARIANT,color4_color3)
    outValue = Color_ctor(inValue.r, inValue.g, inValue.b);
#elif STRING_CMP(VARIANT,color4_vector2)
    outValue = Vec2f_ctor(inValue.r, inValue.g);
#elif STRING_CMP(VARIANT,color4_vector3)
    outValue = Vec3f_ctor(inValue.r, inValue.g, inValue.b);
#elif STRING_CMP(VARIANT,color4_vector4)
    outValue = Vec4f_ctor(inValue.r, inValue.g, inValue.b, inValue.a);
#elif STRING_CMP(VARIANT,vector2_color3)
    outValue = Color_ctor(inValue.x, inValue.y, 0.0);
#elif STRING_CMP(VARIANT,vector2_color4)
    outValue = Col4f_ctor(inValue.x, inValue.y, 0.0, 1.0);
#elif STRING_CMP(VARIANT,vector2_vector3)
    outValue = Vec3f_ctor(inValue.x, inValue.y, 0.0);
#elif STRING_CMP(VARIANT,vector2_vector4)
    outValue = Vec4f_ctor(inValue.x, inValue.y, 0.0, 1.0);
#elif STRING_CMP(VARIANT,vector3_color3)
    outValue = Color_ctor(inValue.x, inValue.y, inValue.z);
#elif STRING_CMP(VARIANT,vector3_color4)
    outValue = Col4f_ctor(inValue.x, inValue.y, inValue.z, 1.0);
#elif STRING_CMP(VARIANT,vector3_vector2)
    outValue = Vec2f_ctor(inValue.x, inValue.y);
#elif STRING_CMP(VARIANT,vector3_vector4)
    outValue = Vec4f_ctor(inValue.x, inValue.y, inValue.z, 1.0);
#elif STRING_CMP(VARIANT,vector4_color3)
    outValue = Color_ctor(inValue.x, inValue.y, inValue.z);
#elif STRING_CMP(VARIANT,vector4_color4)
    outValue = Col4f_ctor(inValue.x, inValue.y, inValue.z, inValue.w);
#elif STRING_CMP(VARIANT,vector4_vector2)
    outValue = Vec2f_ctor(inValue.x, inValue.y);
#elif STRING_CMP(VARIANT,vector4_vector3)
    outValue = Vec3f_ctor(inValue.x, inValue.y, inValue.z);
#elif STRING_CMP(VARIANT,boolean_float)
    outValue = inValue ? 1.0 : 0.0;
#elif STRING_CMP(VARIANT,boolean_color3)
    outValue = inValue ? Color_ctor(1.0, 1.0, 1.0) : Color_ctor(0.0, 0.0, 0.0);
#elif STRING_CMP(VARIANT,boolean_color4)
    outValue = inValue ? Col4f_ctor(1.0, 1.0, 1.0, 1.0) : Col4f_ctor(0.0, 0.0, 0.0, 1.0);
#elif STRING_CMP(VARIANT,boolean_vector2)
    outValue = inValue ? Vec2f_ctor(1.0, 1.0) : Vec2f_ctor(0.0, 0.0);
#elif STRING_CMP(VARIANT,boolean_vector3)
    outValue = inValue ? Vec3f_ctor(1.0, 1.0, 1.0) : Vec3f_ctor(0.0, 0.0, 0.0);
#elif STRING_CMP(VARIANT,boolean_vector4)
    outValue = inValue ? Vec4f_ctor(1.0, 1.0, 1.0, 1.0) : Vec4f_ctor(0.0, 0.0, 0.0, 1.0);
#elif STRING_CMP(VARIANT,boolean_integer)
    outValue = inValue ? 1 : 0;
#elif STRING_CMP(VARIANT,integer_float)
    outValue = inValue;
#elif STRING_CMP(VARIANT,integer_color3)
    outValue = Color_ctor(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,integer_color4)
    outValue = Col4f_ctor(inValue, inValue, inValue, 1.0);
#elif STRING_CMP(VARIANT,integer_vector2)
    outValue = Vec2f_ctor(inValue, inValue);
#elif STRING_CMP(VARIANT,integer_vector3)
    outValue = Vec3f_ctor(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,integer_vector4)
    outValue = Vec4f_ctor(inValue, inValue, inValue, 1.0);
#elif STRING_CMP(VARIANT,integer_boolean)
    outValue = inValue;
#endif

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(OUT_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(OUT_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(OUT_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
