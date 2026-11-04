// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file fractal3d.ispc

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <moonray/common/noise/ispc/Perlin.isph>

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION fractal3d

//
// Configure the variants
// The output type is always the same as OUT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define OUT_TYPE Float
#define OUT_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)
#define OUT_TYPE Vec2f
#define OUT_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)
#define OUT_TYPE Vec3f
#define OUT_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)
#define OUT_TYPE Vec4f
#define OUT_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)
#define OUT_TYPE Color
#define OUT_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)
#define OUT_TYPE Col4f
#define OUT_ATTRKEY Float4AttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

//
// Handle amplitude range types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define AMPLITUDE_TYPE Float
#define AMPLITUDE_ATTRKEY FloatAttrKeyISPC
#else
#define AMPLITUDE_TYPE OUT_TYPE
#define AMPLITUDE_ATTRKEY OUT_ATTRKEY
#endif

#define AMPLITUDE_EVAL CONCAT2(eval,AMPLITUDE_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(OUT_TYPE,Float) || STRING_CMP(OUT_TYPE,Vec2f) || STRING_CMP(OUT_TYPE,Vec3f) || STRING_CMP(OUT_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE OUT_TYPE
#endif

struct SHADER_NAME {
    const NOISE_Perlin *mNoise;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform AMPLITUDE_ATTRKEY* uniform amplitudeAttr;
extern uniform IntAttrKeyISPC* uniform octavesAttr;
extern uniform FloatAttrKeyISPC* uniform lacunarityAttr;
extern uniform FloatAttrKeyISPC* uniform diminishAttr;
extern uniform Float3AttrKeyISPC* uniform positionAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const varying AMPLITUDE_TYPE amplitude = AMPLITUDE_EVAL(map, tls, state, amplitudeAttr);
    const varying int octaves = evalInt(map, tls, state, octavesAttr);
    const varying Float lacunarity = evalFloat(map, tls, state, lacunarityAttr);
    const varying Float diminish = evalFloat(map, tls, state, diminishAttr);
    const varying Vec3f position = evalVec3f(map, tls, state, positionAttr);

    // random position offsets for channels
    const Vec3f Rx = Vec3f_ctor(37.0f,  53.0f,  91.0f);
    const Vec3f Gx = Vec3f_ctor(101.0f, 173.0f, 251.0f);
    const Vec3f Bx = Vec3f_ctor(307.0f, 449.0f, 601.0f);
    const Vec3f Ax = Vec3f_ctor(498.0f, 563.0f, 876.0f);

    varying OUT_TYPE outValue;

#if STRING_CMP(VARIANT,float)
    outValue = NOISE_perlinFractal3D(me->mNoise, position, octaves, diminish, lacunarity) * amplitude;
#elif STRING_CMP(VARIANT,color3)
    outValue.r = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude.r;
    outValue.g = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude.g;
    outValue.b = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude.b;
#elif STRING_CMP(VARIANT,color4)
    outValue.r = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude.r;
    outValue.g = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude.g;
    outValue.b = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude.b;
    outValue.a = NOISE_perlinFractal3D(me->mNoise, position + Ax, octaves, diminish, lacunarity) * amplitude.a;
#elif STRING_CMP(VARIANT,vector2)
    outValue.x = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude.x;
    outValue.y = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude.y;
#elif STRING_CMP(VARIANT,vector3)
    outValue.x = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude.x;
    outValue.y = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude.y;
    outValue.z = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude.z;
#elif STRING_CMP(VARIANT,vector4)
    outValue.x = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude.x;
    outValue.y = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude.y;
    outValue.z = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude.z;
    outValue.w = NOISE_perlinFractal3D(me->mNoise, position + Ax, octaves, diminish, lacunarity) * amplitude.w;
#elif STRING_CMP(VARIANT,color3FA)
    outValue.r = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude;
    outValue.g = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude;
    outValue.b = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude;
#elif STRING_CMP(VARIANT,color4FA)
    outValue.r = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude;
    outValue.g = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude;
    outValue.b = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude;
    outValue.a = NOISE_perlinFractal3D(me->mNoise, position + Ax, octaves, diminish, lacunarity) * amplitude;
#elif STRING_CMP(VARIANT,vector2FA)
    outValue.x = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude;
    outValue.y = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude;
#elif STRING_CMP(VARIANT,vector3FA)
    outValue.x = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude;
    outValue.y = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude;
    outValue.z = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude;
#elif STRING_CMP(VARIANT,vector4FA)
    outValue.x = NOISE_perlinFractal3D(me->mNoise, position + Rx, octaves, diminish, lacunarity) * amplitude;
    outValue.y = NOISE_perlinFractal3D(me->mNoise, position + Gx, octaves, diminish, lacunarity) * amplitude;
    outValue.z = NOISE_perlinFractal3D(me->mNoise, position + Bx, octaves, diminish, lacunarity) * amplitude;
    outValue.w = NOISE_perlinFractal3D(me->mNoise, position + Ax, octaves, diminish, lacunarity) * amplitude;
#endif
        
    //
    // convert to the output type if necessary
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
