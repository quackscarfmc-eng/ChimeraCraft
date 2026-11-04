// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "rgbtohsv.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION rgbtohsv
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

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define RGBTOHSV_TYPE IN_TYPE
#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif


//
// the disableMode flag is initialized in the .cc update() function
// and TRUE indicates that the inValue should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;

varying Color
toHSV(const varying IN_TYPE& rgb)
{
    float cmax = max(rgb.r, max(rgb.g, rgb.b));
    float cmin = min(rgb.r, min(rgb.g, rgb.b));
    float diff = cmax - cmin;

    // Calculate Hue
    float h = 0.f;
    if (diff == 0.f) {
        h = 0.f;
    } else if (cmax == rgb.r) {
        h = fmod(((rgb.g - rgb.b) / diff + 6.f), 6.f);
    } else if (cmax == rgb.g) {
        h = (rgb.b - rgb.r) / diff + 2.f;
    } else {
        h = (rgb.r - rgb.g) / diff + 4.f;
    }
    h /= 6.f;

    // Calculate Saturation
    float s = 0.f;
    if (cmax != 0.f) {
        s = diff / cmax;
    }

    // Calculate Value
    float v = cmax;

    Color hsv;
    hsv.r = h;
    hsv.g = s;
    hsv.b = v;
    return hsv;
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue;
    inValue = IN_EVAL(map, tls, state, inAttr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        outValue = inValue;
    } else {
        Color rgb = toHSV(inValue);
#if STRING_CMP(VARIANT,color3)
        outValue.r = rgb.r;
        outValue.g = rgb.g;
        outValue.b = rgb.b;
#elif STRING_CMP(VARIANT,color4)
        outValue.r = rgb.r;
        outValue.g = rgb.g;
        outValue.b = rgb.b;
        outValue.a = inValue.a;
#endif
    }

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
