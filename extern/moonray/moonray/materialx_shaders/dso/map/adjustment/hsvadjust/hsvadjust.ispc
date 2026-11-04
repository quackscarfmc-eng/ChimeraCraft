// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "hsvadjust.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <scene_rdl2/common/math/ispc/ColorSpace.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION hsvadjust

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
#error VARIANT is not set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)


//
// Handle adjust parameter
//

#define HSVADJUST_TYPE IN_TYPE
#define HSVADJUST_ATTRKEY IN_ATTRKEY

#define HSVADJUST_EVAL CONCAT2(eval,HSVADJUST_TYPE)


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
// disableMode is a flag that gets initialized inValue the .cc update() function
// and TRUE indicates that the in1 value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;
extern uniform HSVADJUST_ATTRKEY* uniform amountAttr;

varying IN_TYPE
hsvAdjust(varying IN_TYPE inValue,
          varying HSVADJUST_TYPE amount)
{
    IN_TYPE outValue = inValue;
    Color inColor = Color_ctor(inValue.r, inValue.g, inValue.b);
    Color hsvColor = rgbToHsv(inColor);
    hsvColor.r += amount.r;
    hsvColor.g *= max(0.0f, amount.g);
    hsvColor.b *= max(0.0f, amount.b);
    Color outColor = hsvToRgb(hsvColor);
    outValue.r = outColor.r;
    outValue.g = outColor.g;
    outValue.b = outColor.b;
    return outValue;
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const varying HSVADJUST_TYPE amount = HSVADJUST_EVAL(map, tls, state, amountAttr);
    varying IN_TYPE inValue = IN_EVAL(map, tls, state, inAttr);

    varying IN_TYPE outValue = inValue;
    if (!me->disableMode) {
        outValue = hsvAdjust(inValue, amount);
    }

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
