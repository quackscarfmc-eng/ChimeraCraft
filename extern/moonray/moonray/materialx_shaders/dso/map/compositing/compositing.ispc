// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "compositing.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following compositing operations with fg, bg, and mix inputs
// use #define OPERATION before including this file
// burn, dodge, screen, difference, plus, minus, overlay
//
// supports the following variants:
// use #define VARIANT before including this file
// float, color3, color4, color3_color3, color4_color4,
// vector2, vector2_vector2, vector3, vector3_vector3, vector4, vector4_vector4
//
// Not every combination of OPERATION and VARIANT is a standard MaterialX shader
// See README for list of shaders.

// OPERATIONS
#define UNIQUE_ID_burn         300
#define UNIQUE_ID_dodge        301
#define UNIQUE_ID_screen       302
#define UNIQUE_ID_difference   303
#define UNIQUE_ID_plus         304
#define UNIQUE_ID_minus        305
#define UNIQUE_ID_overlay      306
#define UNIQUE_ID_in           309
#define UNIQUE_ID_mask         310
#define UNIQUE_ID_matte        311
#define UNIQUE_ID_out          312
#define UNIQUE_ID_over         313
#define UNIQUE_ID_disjointover 314
#define UNIQUE_ID_premult      315
#define UNIQUE_ID_unpremult    316
#define UNIQUE_ID_mix          317

//
// Configure the variants
// The output type is always the same as BG
//
#if STRING_CMP(VARIANT,float)

#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC
#define MIX_TYPE Float
#define MIX_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,color3)

#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define MIX_TYPE Float
#define MIX_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,color4)

#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define MIX_TYPE Float
#define MIX_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,color3_color3)

#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#define MIX_TYPE Color
#define MIX_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,color4_color4)

#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define MIX_TYPE Col4f
#define MIX_ATTRKEY Float4AttrKeyISPC

#elif STRING_CMP(VARIANT,vector2)

#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define MIX_TYPE Float
#define MIX_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2_vector2)

#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define MIX_TYPE Vec2f
#define MIX_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)

#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define MIX_TYPE Float
#define MIX_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector3_vector3)

#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define MIX_TYPE Vec3f
#define MIX_ATTRKEY Float3AttrKeyISPC

#elif STRING_CMP(VARIANT,vector4)

#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define MIX_TYPE Float
#define MIX_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector4_vector4)

#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define MIX_TYPE Vec4f
#define MIX_ATTRKEY Float4AttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)
#define MIX_EVAL CONCAT2(eval,MIX_TYPE)

// Define constructors for zero initialization
#if STRING_CMP(IN_TYPE,Float)
#define IN_TYPE_ctor(x) x
#elif STRING_CMP(IN_TYPE,Color)
#define IN_TYPE_ctor(x) Color_ctor(x, x, x)
#elif STRING_CMP(IN_TYPE,Col4f)
#define IN_TYPE_ctor(x) Col4f_ctor(x, x, x, x)
#elif STRING_CMP(IN_TYPE,Vec2f)
#define IN_TYPE_ctor(x) Vec2f_ctor(x, x)
#elif STRING_CMP(IN_TYPE,Vec3f)
#define IN_TYPE_ctor(x) Vec3f_ctor(x, x, x)
#elif STRING_CMP(IN_TYPE,Vec4f)
#define IN_TYPE_ctor(x) Vec4f_ctor(x, x, x, x)
#endif

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Color) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f)
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

static varying
float doOperation(varying float& fg, varying float& bg)
{
#if STRING_CMP(OPERATION,burn)
    return fg < sEpsilon ? 0.0f : 1.0f - (1.0f - bg)/fg;
#elif STRING_CMP(OPERATION,dodge)
    return abs(1.0f - fg) < sEpsilon ? 0.0f : bg/(1.0f - fg);
#elif STRING_CMP(OPERATION,screen)
    return 1.0f - (1.0f - fg) * (1.0f - bg);
#elif STRING_CMP(OPERATION,difference)
    return abs(fg - bg);
#elif STRING_CMP(OPERATION,plus)
    return fg + bg;
#elif STRING_CMP(OPERATION,minus)
    return bg - fg;
#elif STRING_CMP(OPERATION,overlay)
    return bg < 0.5f ? 2.0f * fg * bg : 1.0f - 2.0f * (1.0f - fg) * (1.0f - bg);
#elif STRING_CMP(OPERATION,in)
    return fg * bg;
#elif STRING_CMP(OPERATION,mask)
    return fg * bg;
#elif STRING_CMP(OPERATION,matte)
    return fg + bg * (1.0f - fg);
#elif STRING_CMP(OPERATION,out)
    return fg * (1.0f - bg);
#elif STRING_CMP(OPERATION,over)
    return fg + bg * (1.0f - fg);
#elif STRING_CMP(OPERATION,disjointover)
    return fg + bg;
#elif STRING_CMP(OPERATION,premult)
    return fg * bg;
#elif STRING_CMP(OPERATION,unpremult)
    return bg < sEpsilon ? 0.0f : fg / bg;
#elif STRING_CMP(OPERATION,mix)
    return fg;
#elif defined(OPERATION)
    #error OPERATION is set to an invalid value
#else
    #error OPERATION isn't set
#endif
}

#if STRING_CMP(OPERATION,premult) || STRING_CMP(OPERATION,unpremult)
// premult/unpremult use single 'in' parameter
extern uniform IN_ATTRKEY* uniform inAttr;
#else
// All other operations use 'fg', 'bg', and 'mix'
extern uniform IN_ATTRKEY* uniform fgAttr;
extern uniform IN_ATTRKEY* uniform bgAttr;
extern uniform MIX_ATTRKEY* uniform mixAttr;
#endif

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE fgValue;
    varying IN_TYPE bgValue;
    varying MIX_TYPE mixValue;

#if STRING_CMP(OPERATION,premult) || STRING_CMP(OPERATION,unpremult)
    // premult/unpremult use single 'in' parameter
    fgValue = IN_EVAL(map, tls, state, inAttr);
    bgValue = IN_TYPE_ctor(0);  // Not used
    mixValue = 1.0f;  // No mix parameter for premult/unpremult
#else
    // All other operations use 'fg', 'bg', and 'mix'
    fgValue = IN_EVAL(map, tls, state, fgAttr);
    bgValue = IN_EVAL(map, tls, state, bgAttr);
    mixValue = MIX_EVAL(map, tls, state, mixAttr);
#endif

    varying IN_TYPE outValue;
    if (me->disableMode) {
#if STRING_CMP(OPERATION,premult) || STRING_CMP(OPERATION,unpremult)
        // For premult/unpremult, disable returns input
        outValue = fgValue;
#else
        // when it's disabled then it just passes through the bg value unchanged
        outValue = bgValue;
#endif
    } else {

//
// doOperation is just a single float but the compiler should be able
// optimize out all of the function calls.
//
#if STRING_CMP(VARIANT,float)
        varying float tmp = doOperation(fgValue, bgValue);
        outValue = lerp(bgValue, tmp, mixValue);
        // mimic Karma behavior by clamping negative values post-lerp
        outValue = max(0.0f, outValue);
#elif STRING_CMP(VARIANT,color3)
        varying Color tmp;
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        outValue = lerp(bgValue, tmp, mixValue);
        // mimic Karma behavior by clamping negative values post-lerp
        outValue.r = max(0.0f, outValue.r);
        outValue.g = max(0.0f, outValue.g);
        outValue.b = max(0.0f, outValue.b);
#elif STRING_CMP(VARIANT,color3_color3)
        varying Color tmp;
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        outValue.r = lerp(bgValue.r, tmp.r, mixValue.r);
        outValue.g = lerp(bgValue.g, tmp.g, mixValue.g);
        outValue.b = lerp(bgValue.b, tmp.b, mixValue.b);
        // mimic Karma behavior by clamping negative values post-lerp
        outValue.r = max(0.0f, outValue.r);
        outValue.g = max(0.0f, outValue.g);
        outValue.b = max(0.0f, outValue.b);

#elif STRING_CMP(VARIANT,color4)
        // Color4 operations have different behaviors depending on the operation type
        varying Col4f tmp;

        // Operations with alpha compositing (use whole color4 with alpha as scalar):
        // in, mask, out, over
#if STRING_CMP(OPERATION,in)
        // Reference: (fg * bg.a * mix) + (bg * (1-mix))
        tmp = fgValue * bgValue.a;
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,mask)
        // Reference: (bg * fg.a * mix) + (bg * (1-mix))
        tmp = bgValue * fgValue.a;
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,out)
        // Reference: (fg * (1-bg.a) * mix) + (bg * (1-mix))
        tmp = fgValue * (1.0f - bgValue.a);
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,over)
        // Reference: (fg + bg*(1-fg.a)) * mix + bg * (1-mix)
        tmp = fgValue + bgValue * (1.0f - fgValue.a);
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,matte)
        // Reference: color4(fg.rgb*fg.a + bg.rgb*(1-fg.a), fg.a + bg.a*(1-fg.a)) * mix + bg * (1-mix)
        tmp.r = fgValue.r * fgValue.a + bgValue.r * (1.0f - fgValue.a);
        tmp.g = fgValue.g * fgValue.a + bgValue.g * (1.0f - fgValue.a);
        tmp.b = fgValue.b * fgValue.a + bgValue.b * (1.0f - fgValue.a);
        tmp.a = fgValue.a + bgValue.a * (1.0f - fgValue.a);
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,disjointover)
        // Reference: complex conditional based on summed alpha
        varying float summedAlpha = fgValue.a + bgValue.a;
        if (summedAlpha <= 1.0f) {
            tmp.r = fgValue.r + bgValue.r;
            tmp.g = fgValue.g + bgValue.g;
            tmp.b = fgValue.b + bgValue.b;
        } else {
            if (abs(bgValue.a) < sEpsilon) {
                tmp.r = 0.0f;
                tmp.g = 0.0f;
                tmp.b = 0.0f;
            } else {
                varying float x = (1.0f - fgValue.a) / bgValue.a;
                tmp.r = fgValue.r + bgValue.r * x;
                tmp.g = fgValue.g + bgValue.g * x;
                tmp.b = fgValue.b + bgValue.b * x;
            }
        }
        tmp.a = min(summedAlpha, 1.0f);
        outValue.r = tmp.r * mixValue + bgValue.r * (1.0f - mixValue);
        outValue.g = tmp.g * mixValue + bgValue.g * (1.0f - mixValue);
        outValue.b = tmp.b * mixValue + bgValue.b * (1.0f - mixValue);
        outValue.a = tmp.a * mixValue + bgValue.a * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,premult)
        // Reference: color4(fg.rgb * fg.a, fg.a) - no mix parameter in reference!
        outValue.r = fgValue.r * fgValue.a;
        outValue.g = fgValue.g * fgValue.a;
        outValue.b = fgValue.b * fgValue.a;
        outValue.a = fgValue.a;

#elif STRING_CMP(OPERATION,unpremult)
        // Reference: color4(fg.rgb / fg.a, fg.a) - no mix parameter in reference!
        if (abs(fgValue.a) < sEpsilon) {
            outValue.r = 0.0f;
            outValue.g = 0.0f;
            outValue.b = 0.0f;
        } else {
            outValue.r = fgValue.r / fgValue.a;
            outValue.g = fgValue.g / fgValue.a;
            outValue.b = fgValue.b / fgValue.a;
        }
        outValue.a = fgValue.a;

#else
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        tmp.a = doOperation(fgValue.a, bgValue.a);
        outValue = lerp(bgValue, tmp, mixValue);
#endif
        // mimic Karma behavior by clamping negative values post-lerp (except for premult/unpremult which don't use lerp)
#if !STRING_CMP(OPERATION,premult) && !STRING_CMP(OPERATION,unpremult)
        outValue.r = max(0.0f, outValue.r);
        outValue.g = max(0.0f, outValue.g);
        outValue.b = max(0.0f, outValue.b);
        outValue.a = max(0.0f, outValue.a);
#endif

#elif STRING_CMP(VARIANT,color4_color4)
        // Same as color4 variant
        varying Col4f tmp;
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        tmp.a = doOperation(fgValue.a, bgValue.a);
        outValue.r = max(0.0f, lerp(bgValue.r, tmp.r, mixValue.r));
        outValue.g = max(0.0f, lerp(bgValue.g, tmp.g, mixValue.g));
        outValue.b = max(0.0f, lerp(bgValue.b, tmp.b, mixValue.b));
        outValue.a = max(0.0f, lerp(bgValue.a, tmp.a, mixValue.a));

#elif STRING_CMP(VARIANT,vector2)
        varying Vec2f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        outValue = lerp(bgValue, tmp, mixValue);
#elif STRING_CMP(VARIANT,vector2_vector2)
        varying Vec2f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        outValue.r = lerp(bgValue.x, tmp.x, mixValue.x);
        outValue.g = lerp(bgValue.y, tmp.y, mixValue.y);
#elif STRING_CMP(VARIANT,vector3)
        varying Vec3f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        outValue = lerp(bgValue, tmp, mixValue);
#elif STRING_CMP(VARIANT,vector3_vector3)
        varying Vec3f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        outValue.r = lerp(bgValue.x, tmp.x, mixValue.x);
        outValue.g = lerp(bgValue.y, tmp.y, mixValue.y);
        outValue.b = lerp(bgValue.z, tmp.z, mixValue.z);
#elif STRING_CMP(VARIANT,vector4)
        varying Vec4f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        tmp.w = doOperation(fgValue.w, bgValue.w);
        outValue = lerp(bgValue, tmp, mixValue);
#elif STRING_CMP(VARIANT,vector4_vector4)
        varying Vec4f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        tmp.w = doOperation(fgValue.w, bgValue.w);
        outValue.r = lerp(bgValue.x, tmp.x, mixValue.x);
        outValue.g = lerp(bgValue.y, tmp.y, mixValue.y);
        outValue.b = lerp(bgValue.z, tmp.z, mixValue.z);
        outValue.a = lerp(bgValue.w, tmp.w, mixValue.w);

#endif
    }

    //
    // convert the return value if needed
    //
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
