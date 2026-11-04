// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "adobergb_to_lin_rec709.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <scene_rdl2/common/math/ispc/ColorSpace.isph>
#include <scene_rdl2/common/math/ispc/Math.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION adobergb_to_lin_rec709

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
// disableMode is a flag that gets initialized in the .cc update() function
// and TRUE indicates that the in value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;

varying IN_TYPE
doAdobergbToLinRec709(varying IN_TYPE inValue)
{
    // Adobe RGB to Linear Rec.709 transformation
    // From MaterialX: https://github.com/AcademySoftwareFoundation/MaterialX
    // 
    // Step 1: Apply inverse gamma (563/256 ≈ 2.19921875) to linearize Adobe RGB
    const uniform float gamma = 563.0f / 256.0f;
    
    IN_TYPE linearAdobeRgb = inValue;
    linearAdobeRgb.r = max(0.0f, inValue.r);
    linearAdobeRgb.g = max(0.0f, inValue.g);
    linearAdobeRgb.b = max(0.0f, inValue.b);
    
    linearAdobeRgb.r = pow(linearAdobeRgb.r, gamma);
    linearAdobeRgb.g = pow(linearAdobeRgb.g, gamma);
    linearAdobeRgb.b = pow(linearAdobeRgb.b, gamma);
    
    // Adobe RGB and Rec.709 share the same primaries (both D65 white point)
    // so no matrix transformation is needed, only gamma decode
    return linearAdobeRgb;
}

varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue = IN_EVAL(map, tls, state, inAttr);

    varying IN_TYPE outValue = inValue;
    if (!me->disableMode) {
        outValue = doAdobergbToLinRec709(inValue);
    }

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
