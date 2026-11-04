// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file hsvtorgb.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

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

#define HSVTORGB_TYPE IN_TYPE
#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types being mapped to Color
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
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
    // the function pointers on Map are unions so it doesn't
    // really matter which one is assigned to and the variability
    // is easier to manage with macros with picking one and forcing
    // a cast
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

Color
toRGB(const IN_TYPE& hsv)
{
    float h = hsv.r;
    float s = hsv.g;
    float v = hsv.b;

    float h6 = h * 6;
    float h6Floor = std::floor(h6);
    float f = h6 - h6Floor;

    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    int i = static_cast<int>(h6Floor);

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

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
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

    *sample = outValue;
}
