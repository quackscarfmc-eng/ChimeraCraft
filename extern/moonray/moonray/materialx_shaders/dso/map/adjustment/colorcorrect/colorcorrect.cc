// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file colorcorrect.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

#include <scene_rdl2/common/math/ColorSpace.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION colorcorrect
#define IN_DEFAULT_FLOAT 0.0f
#define HUE_DEFAULT_FLOAT 0.0f
#define SATURATION_DEFAULT_FLOAT 1.0f
#define GAMMA_DEFAULT_FLOAT 1.0f
#define LIFT_DEFAULT_FLOAT 0.0f
#define GAIN_DEFAULT_FLOAT 1.0f
#define CONTRAST_DEFAULT_FLOAT 1.0f
#define CONTRASTPIVOT_DEFAULT_FLOAT 0.5f
#define EXPOSURE_DEFAULT_FLOAT 0.0f

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_DEFAULT Color(1.0f, 1.0f, 1.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(1.0f, 1.0f, 1.0f, 0.0f)
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif


//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<IN_TYPE> inAttr;
    AttributeKey<Float> hueAttr;
    AttributeKey<Float> saturationAttr;
    AttributeKey<Float> gammaAttr;
    AttributeKey<Float> liftAttr;
    AttributeKey<Float> gainAttr;
    AttributeKey<Float> contrastAttr;
    AttributeKey<Float> contrastpivotAttr;
    AttributeKey<Float> exposureAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    hueAttr = sceneClass.declareAttribute<Float>("hue", HUE_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    saturationAttr = sceneClass.declareAttribute<Float>("saturation", SATURATION_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    gammaAttr = sceneClass.declareAttribute<Float>("gamma", GAMMA_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    liftAttr = sceneClass.declareAttribute<Float>("lift", LIFT_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    gainAttr = sceneClass.declareAttribute<Float>("gain", GAIN_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    contrastAttr = sceneClass.declareAttribute<Float>("contrast", CONTRAST_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    contrastpivotAttr = sceneClass.declareAttribute<Float>("contrastpivot", CONTRASTPIVOT_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    exposureAttr = sceneClass.declareAttribute<Float>("exposure", EXPOSURE_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END


//
// end of attributes.cc
//

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, RETURN_TYPE *sample);
    ispc::SHADER_NAME mIspc;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
                         const std::string& name) :
    Parent(sceneClass, name)
{
    // the functions pointers on Map are unions so it doesn't
    // really matter which one is assigned to and the variability
    // is easier to manage with macros with picking one and forcing
    // with a cast
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

IN_TYPE
colorCorrect(IN_TYPE inValue,
             Float hue,
             Float saturation,
             Float gamma,
             Float lift,
             Float gain,
             Float contrast,
             Float contrastPivot,
             Float exposure)
{
    IN_TYPE outValue = inValue;
    
    // Extract RGB components for color3/color4
    Color inColor(outValue.r, outValue.g, outValue.b);
    
    // Apply exposure (2^value)
    if (exposure != 0.0f) {
        Float exposureFactor = pow(2.0f, exposure);
        inColor.r *= exposureFactor;
        inColor.g *= exposureFactor;
        inColor.b *= exposureFactor;
    }
    
    // Apply lift and gain
    if (lift != 0.0f || gain != 1.0f) {
        inColor.r = lift + inColor.r * gain;
        inColor.g = lift + inColor.g * gain;
        inColor.b = lift + inColor.b * gain;
    }
    
    // Apply gamma correction
    if (gamma != 1.0f && gamma > 0.0f) {
        Float invGamma = 1.0f / gamma;
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

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const Float hue = evalFloat(me, hueAttr, tls, state);
    const Float saturation = evalFloat(me, saturationAttr, tls, state);
    const Float gamma = evalFloat(me, gammaAttr, tls, state);
    const Float lift = evalFloat(me, liftAttr, tls, state);
    const Float gain = evalFloat(me, gainAttr, tls, state);
    const Float contrast = evalFloat(me, contrastAttr, tls, state);
    const Float contrastPivot = evalFloat(me, contrastpivotAttr, tls, state);
    const Float exposure = evalFloat(me, exposureAttr, tls, state);
    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);

    IN_TYPE outValue = inValue;
    if (!me->mIspc.disableMode) {
        outValue = colorCorrect(inValue, hue, saturation, gamma, lift, gain, 
                               contrast, contrastPivot, exposure);
    }

    *sample = outValue;
}
