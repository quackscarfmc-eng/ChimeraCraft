// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "colorcorrect.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <scene_rdl2/common/math/ispc/ColorSpace.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION colorcorrect

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
extern uniform FloatAttrKeyISPC* uniform hueAttr;
extern uniform FloatAttrKeyISPC* uniform saturationAttr;
extern uniform FloatAttrKeyISPC* uniform gammaAttr;
extern uniform FloatAttrKeyISPC* uniform liftAttr;
extern uniform FloatAttrKeyISPC* uniform gainAttr;
extern uniform FloatAttrKeyISPC* uniform contrastAttr;
extern uniform FloatAttrKeyISPC* uniform contrastpivotAttr;
extern uniform FloatAttrKeyISPC* uniform exposureAttr;

varying IN_TYPE
colorCorrect(varying IN_TYPE inValue,
             varying float hue,
             varying float saturation,
             varying float gamma,
             varying float lift,
             varying float gainVal,
             varying float contrast,
             varying float contrastPivot,
             varying float exposure)
{
    IN_TYPE outValue = inValue;
    
    // Extract RGB components for color3/color4
    Color inColor = Color_ctor(outValue.r, outValue.g, outValue.b);
    
    // Apply exposure (2^value)
    if (exposure != 0.0f) {
        float exposureFactor = pow(2.0f, exposure);
        inColor.r *= exposureFactor;
        inColor.g *= exposureFactor;
        inColor.b *= exposureFactor;
    }
    
    // Apply lift and gainVal
    if (lift != 0.0f || gainVal != 1.0f) {
        inColor.r = lift + inColor.r * gainVal;
        inColor.g = lift + inColor.g * gainVal;
        inColor.b = lift + inColor.b * gainVal;
    }
    
    // Apply gamma correction
    if (gamma != 1.0f && gamma > 0.0f) {
        float invGamma = 1.0f / gamma;
        inColor.r = max(0.0f, pow(max(0.0f, inColor.r), invGamma));
        inColor.g = max(0.0f, pow(max(0.0f, inColor.g), invGamma));
        inColor.b = max(0.0f, pow(max(0.0f, inColor.b), invGamma));
    }
    
    // Apply contrast around pivot
    if (contrast != 1.0f) {
        inColor.r = contrastPivot + (inColor.r - contrastPivot) * contrast;
        inColor.g = contrastPivot + (inColor.g - contrastPivot) * contrast;
        inColor.b = contrastPivot + (inColor.b - contrastPivot) * contrast;
    }
    
    // Apply hue and saturation adjustments in HSV space
    if (hue != 0.0f || saturation != 1.0f) {
        Color hsvColor = rgbToHsv(inColor);
        hsvColor.r += hue; // Hue adjustment (wraps at 0-1 boundaries)
        hsvColor.g *= max(0.0f, saturation); // Saturation adjustment
        inColor = hsvToRgb(hsvColor);
    }
    
    // Set the result back to output
    outValue.r = inColor.r;
    outValue.g = inColor.g;
    outValue.b = inColor.b;
    // For color4, alpha channel remains unchanged
    
    return outValue;
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const varying float hue = evalFloat(map, tls, state, hueAttr);
    const varying float saturation = evalFloat(map, tls, state, saturationAttr);
    const varying float gamma = evalFloat(map, tls, state, gammaAttr);
    const varying float lift = evalFloat(map, tls, state, liftAttr);
    const varying float gainVal = evalFloat(map, tls, state, gainAttr);
    const varying float contrast = evalFloat(map, tls, state, contrastAttr);
    const varying float contrastPivot = evalFloat(map, tls, state, contrastpivotAttr);
    const varying float exposure = evalFloat(map, tls, state, exposureAttr);
    varying IN_TYPE inValue = IN_EVAL(map, tls, state, inAttr);

    varying IN_TYPE outValue = inValue;
    if (!me->disableMode) {
        outValue = colorCorrect(inValue, hue, saturation, gamma, lift, gainVal, 
                                contrast, contrastPivot, exposure);
    }

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
