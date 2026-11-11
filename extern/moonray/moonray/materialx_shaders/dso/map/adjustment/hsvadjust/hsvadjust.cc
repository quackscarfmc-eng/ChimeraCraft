// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file hsvadjust.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

#include <scene_rdl2/common/math/ColorSpace.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION hsvadjust
#define IN_DEFAULT_FLOAT 0.0f
#define AMOUNT_DEFAULT_R 0.0f
#define AMOUNT_DEFAULT_G 1.0f
#define AMOUNT_DEFAULT_B 1.0f
#define AMOUNT_DEFAULT_A 1.0f

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Handle adjust parameter
//

#define HSVADJUST_TYPE IN_TYPE

#define HSVADJUST_EVAL CONCAT2(eval,HSVADJUST_TYPE)

#if STRING_CMP(VARIANT,color3)
#define HSVADJUST_AMOUNT_DEFAULT Color(AMOUNT_DEFAULT_R,AMOUNT_DEFAULT_G,AMOUNT_DEFAULT_B)

#elif STRING_CMP(VARIANT,color4)
#define HSVADJUST_AMOUNT_DEFAULT Rgba(AMOUNT_DEFAULT_R,AMOUNT_DEFAULT_G,AMOUNT_DEFAULT_B,AMOUNT_DEFAULT_A)

#endif

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
    AttributeKey<HSVADJUST_TYPE> amountAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    amountAttr = sceneClass.declareAttribute<HSVADJUST_TYPE>("amount", HSVADJUST_AMOUNT_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
hsvAdjust(IN_TYPE inValue,
          HSVADJUST_TYPE amount)
{
    IN_TYPE outValue = inValue;
    Color inColor(inValue.r, inValue.g, inValue.b);
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

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const HSVADJUST_TYPE amount = HSVADJUST_EVAL(me, amountAttr, tls, state);
    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);

    IN_TYPE outValue = inValue;
    if (!me->mIspc.disableMode) {
        outValue = hsvAdjust(inValue, amount);
    }

    *sample = outValue;
}

