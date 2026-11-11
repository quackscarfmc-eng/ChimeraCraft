// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "clamp.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <moonray/rendering/shading/ispc/BasicTexture.isph>
#include <moonray/rendering/shading/ispc/UdimTexture.isph>
#include <moonray/rendering/shading/ispc/Shading.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>

// Supports the following variants:
//  - float
//  - color3
//  - color4
//  - vector2
//  - vector3
//  - vector4

#define OPERATION image

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define OUT_DEFAULT Vec4f_ctor(0.0f, 0.0f, 0.0f, 0.0f)

#elif STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC

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


#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#define OUT_DEFAULT Color_ctor(0.0f, 0.0f, 0.0f)
#else
#define RETURN_TYPE IN_TYPE
#endif


enum AddressModeType {
    CONSTANT = 0,
    CLAMP = 1,
    PERIODIC = 2,
    MIRROR = 3,
};
ISPC_UTIL_EXPORT_ENUM_TO_HEADER(AddressModeType);

struct SHADER_NAME {
    const uniform BASIC_TEXTURE_Data * uniform mTexture;
    const uniform UDIM_TEXTURE_Data * uniform mUdimTexture;
    uniform bool mDisableMode;
    uniform bool mTexCoordUseDefault;
    uniform Color mFatalColor;
    uniform Color mDefaultColor;
    uniform AddressModeType mUAddressMode;
    uniform AddressModeType mVAddressMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform BoolAttrKeyISPC* uniform disableAttr;
extern uniform IN_ATTRKEY* uniform defaultAttr;
extern uniform IntAttrKeyISPC* uniform filtertypeAttr;
extern uniform IntAttrKeyISPC* uniform frameendactionAttr;
extern uniform IntAttrKeyISPC* uniform frameoffsetAttr;
extern uniform Float2AttrKeyISPC* uniform texcoordAttr;
extern uniform IntAttrKeyISPC* uniform uaddressmodeAttr;
extern uniform IntAttrKeyISPC* uniform vaddressmodeAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    if (me->mDisableMode) {
        return OUT_DEFAULT;
    }

    if (!me->mTexture) {
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
        return me->mFatalColor;
#else
        return OUT_DEFAULT;
#endif
    }

    varying Vec2f texcoordValue;
    if (me->mTexCoordUseDefault) {
        texcoordValue = getSt(state);
    } else {
        texcoordValue = evalVec2f(map, tls, state, texcoordAttr);
    }


    varying float derivatives[4] = {0.f, 0.f, 0.f, 0.f};

    switch(me->mUAddressMode) {
    case(MIRROR):
        if (texcoordValue.x < 0.f) {
            texcoordValue.x = -texcoordValue.x;
            derivatives[0] = -getdSdx(state);
            derivatives[2] = -getdSdy(state);
        } else {
            derivatives[0] = getdSdx(state);
            derivatives[2] = getdSdy(state);
        }
        break;
    default:
        derivatives[0] = getdSdx(state);
        derivatives[2] = getdSdy(state);
        break;
    }

    switch(me->mVAddressMode) {
    case(MIRROR):
        if (texcoordValue.y < 0.f) {
            texcoordValue.y = -texcoordValue.y;
            derivatives[1] = getdTdx(state);
            derivatives[3] = getdTdy(state);
        } else {
            derivatives[1] = -getdTdx(state);
            derivatives[3] = -getdTdy(state);
        }
        break;
    default:
        derivatives[1] = -getdTdx(state);
        derivatives[3] = -getdTdy(state);
        break;
    }

    varying Vec2f st = texcoordValue;
    varying Col4f tx;

    if (me->mUdimTexture) {
        varying int udim = UDIM_TEXTURE_compute_udim(me->mUdimTexture, tls, st);
        if (udim == -1) {
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
        return me->mFatalColor;
#else
        return OUT_DEFAULT;
#endif
        }
        st.x = st.x - (int)st.x;
        st.y = st.y - (int)st.y;
        // OIIO texture flip
        st.y = 1.f - st.y;

        tx = UDIM_TEXTURE_sample(me->mUdimTexture,
                                 tls,
                                 state,
                                 udim,
                                 st,
                                 derivatives);
    } else {
        // OIIO texture flip
        st.y = 1.f - st.y;

        tx = BASIC_TEXTURE_sample(me->mTexture,
                                  tls,
                                  state,
                                  st,
                                  derivatives);
    }

    IN_TYPE outValue;
#if STRING_CMP(VARIANT,float)
    outValue = tx.r;
#elif STRING_CMP(VARIANT,color3)
    outValue.r = tx.r;
    outValue.g = tx.g;
    outValue.b = tx.b;
#elif STRING_CMP(VARIANT,color4)
    outValue = tx;
#elif STRING_CMP(VARIANT,vector2)
    outValue.x = tx.r;
    outValue.y = tx.g;
#elif STRING_CMP(VARIANT,vector3)
    outValue.x = tx.r;
    outValue.y = tx.g;
    outValue.z = tx.b;
#elif STRING_CMP(VARIANT,vector4)
    outValue.x = tx.r;
    outValue.y = tx.g;
    outValue.z = tx.b;
    outValue.w = tx.a;
#endif

#if STRING_CMP(IN_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
