// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "hextiledimage.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <materialx_shaders/map/ispc/MtlxHextile.isph>
#include <moonray/rendering/shading/ispc/BasicTexture.isph>
#include <moonray/rendering/shading/ispc/Shading.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>

// Supports the following variants:
//  - color3
//  - color4

#define OPERATION hextiledimage

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define OUT_DEFAULT Color_ctor(0.0f, 0.0f, 0.0f)

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_DEFAULT Col4f_ctor(0.0f, 0.0f, 0.0f, 0.0f)

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)


#define RETURN_TYPE IN_TYPE


struct SHADER_NAME {
    const uniform BASIC_TEXTURE_Data * uniform mTexture;
    uniform bool mDisableMode;
    uniform bool mTexCoordUseDefault;
    uniform Color mFatalColor;
    uniform Color mDefaultColor;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform BoolAttrKeyISPC* uniform disableAttr;
extern uniform IN_ATTRKEY* uniform defaultAttr;
extern uniform Float2AttrKeyISPC* uniform texcoordAttr;
extern uniform FloatAttrKeyISPC* uniform falloffAttr;
extern uniform FloatAttrKeyISPC* uniform falloffcontrastAttr;
extern uniform FloatAttrKeyISPC* uniform offsetAttr;
extern uniform Float2AttrKeyISPC* uniform offsetrangeAttr;
extern uniform FloatAttrKeyISPC* uniform rotationAttr;
extern uniform Float2AttrKeyISPC* uniform rotationrangeAttr;
extern uniform FloatAttrKeyISPC* uniform scaleAttr;
extern uniform Float2AttrKeyISPC* uniform scalerangeAttr;
extern uniform Float2AttrKeyISPC* uniform tilingAttr;

static varying RETURN_TYPE
sample(const uniform Map*    uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    if (me->mDisableMode) {
        return OUT_DEFAULT;
    }

    if (!me->mTexture) {
#if STRING_CMP(VARIANT,color4)
        const uniform Color errColor = me->mFatalColor;
        return Col4f_ctor(errColor.r, errColor.g, errColor.b, 1.0f);
#else
        return me->mFatalColor;
#endif
    }

    varying Vec2f texcoordValue;
    if (me->mTexCoordUseDefault) {
        texcoordValue = getSt(state);
    } else {
        texcoordValue = evalVec2f(map, tls, state, texcoordAttr);
    }

    const varying Vec2f tiling = evalVec2f(map, tls, state, tilingAttr);

    // Generate 3 texture coordinates & respective derivatives on hexagonal grid
    varying HextileData tiledData;
    HextileCoord(texcoordValue * tiling,
                 evalFloat(map, tls, state, rotationAttr),
                 evalVec2f(map, tls, state, rotationrangeAttr),
                 evalFloat(map, tls, state, scaleAttr),
                 evalVec2f(map, tls, state, scalerangeAttr),
                 evalFloat(map, tls, state, offsetAttr),
                 evalVec2f(map, tls, state, offsetrangeAttr),
                 Vec2f_ctor(getdSdx(state), -getdTdx(state)),
                 Vec2f_ctor(getdSdy(state), -getdTdy(state)),
                 tiledData);

    // For each coordinate, sample the texture
    varying float d0[4] = {0.f, 0.f, 0.f, 0.f};
    d0[0] = tiledData.ddx[0].x;
    d0[1] = tiledData.ddy[0].x;
    d0[2] = tiledData.ddx[0].y;
    d0[3] = tiledData.ddy[0].y;
    tiledData.coords[0].y = 1.0f - tiledData.coords[0].y;
    const varying Col4f tx0 = BASIC_TEXTURE_sample(me->mTexture,
                                                    tls, state,
                                                    tiledData.coords[0],
                                                    d0);
    const varying Vec3f tx0v = Vec3f_ctor(tx0.r, tx0.g, tx0.b);

    varying float d1[4] = {0.f, 0.f, 0.f, 0.f};
    d1[0] = tiledData.ddx[1].x;
    d1[1] = tiledData.ddy[1].x;
    d1[2] = tiledData.ddx[1].y;
    d1[3] = tiledData.ddy[1].y;
    tiledData.coords[1].y = 1.0f - tiledData.coords[1].y;
    const varying Col4f tx1 = BASIC_TEXTURE_sample(me->mTexture,
                                                    tls, state,
                                                    tiledData.coords[1],
                                                    d1);
    const varying Vec3f tx1v = Vec3f_ctor(tx1.r, tx1.g, tx1.b);

    varying float d2[4] = {0.f, 0.f, 0.f, 0.f};
    d2[0] = tiledData.ddx[2].x;
    d2[1] = tiledData.ddy[2].x;
    d2[2] = tiledData.ddx[2].y;
    d2[3] = tiledData.ddy[2].y;
    tiledData.coords[2].y = 1.0f - tiledData.coords[2].y;
    const varying Col4f tx2 = BASIC_TEXTURE_sample(me->mTexture,
                                                    tls, state,
                                                    tiledData.coords[2],
                                                    d2);
    const varying Vec3f tx2v = Vec3f_ctor(tx2.r, tx2.g, tx2.b);

    const varying Vec3f lumacoeffs = Vec3f_ctor(0.2722287, 0.6740818, 0.0536895);
    varying Vec3f cw = Vec3f_ctor(dot(tx0v, lumacoeffs),
                                  dot(tx1v, lumacoeffs),
                                  dot(tx2v, lumacoeffs));
    const varying float falloff_contrast = evalFloat(map, tls, state, falloffcontrastAttr);
    cw = lerp(Vec3f_ctor(1.0f), cw, falloff_contrast);

    const varying float falloff = evalFloat(map, tls, state, falloffAttr);
    const varying Vec3f w = HextileComputeBlendWeights(cw, tiledData.weights, falloff);

    Col4f outColor = w.x * tx0 + w.y * tx1 + w.z * tx2;

    IN_TYPE outValue;
#if STRING_CMP(VARIANT,color3)
    outValue.r = outColor.r;
    outValue.g = outColor.g;
    outValue.b = outColor.b;
#elif STRING_CMP(VARIANT,color4)
    outColor.a = (tx0.a + tx1.a + tx2.a) / 3.0f;
    outValue = outColor;
#endif

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)

