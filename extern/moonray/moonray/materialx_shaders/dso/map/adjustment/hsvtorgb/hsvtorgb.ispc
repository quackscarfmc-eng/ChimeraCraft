// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "hsvtorgb.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION hsvtorgb
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

#define HSVTORGB_TYPE IN_TYPE
#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types being mapped to Color
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
toRGB(const varying IN_TYPE& hsv)
{
    float h = hsv.r;
    float s = hsv.g;
    float v = hsv.b;

    float h6 = h * 6;
    float h6Floor = floor(h6);
    float f = h6 - h6Floor;

    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    int i = (int)h6Floor;

    Color rgb;

    switch (i % 6) {
        case 0: rgb.r = v; rgb.g = t; rgb.b = p; break;
        case 1: rgb.r = q; rgb.g = v; rgb.b = p; break;
        case 2: rgb.r = p; rgb.g = v; rgb.b = t; break;
        case 3: rgb.r = p; rgb.g = q; rgb.b = v; break;
        case 4: rgb.r = t; rgb.g = p; rgb.b = v; break;
        case 5: rgb.r = v; rgb.g = p; rgb.b = q; break;
    }

    return rgb;
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
        Color rgb = toRGB(inValue);
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
