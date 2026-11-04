// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file invert.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION invert
#define IN_DEFAULT_FLOAT 0.0f
#define AMOUNT_DEFAULT_FLOAT 1.0f

//
// Configure the variants
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)
#define IN_TYPE Vec2f
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)
#define IN_TYPE Vec3f
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)
#define IN_TYPE Vec4f
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)
#define IN_TYPE Color
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)
#define IN_TYPE Rgba
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

// Once we've found the IN_TYPE, we can more easily find the other defaults
#if STRING_CMP(VARIANT, Float)
#define AMOUNT_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define AMOUNT_DEFAULT AMOUNT_DEFAULT_FLOAT

#elif STRING_CMP(VARIANT, vector2FA) || STRING_CMP(VARIANT, vector3FA) || STRING_CMP(VARIANT, vector4FA) || \
      STRING_CMP(VARIANT, color3FA)  || STRING_CMP(VARIANT, color4FA)
#define AMOUNT_TYPE Float
#define IN_DEFAULT IN_TYPE(IN_DEFAULT_FLOAT)
#define AMOUNT_DEFAULT AMOUNT_DEFAULT_FLOAT

#else
#define AMOUNT_TYPE IN_TYPE
#define IN_DEFAULT IN_TYPE(IN_DEFAULT_FLOAT)
#define AMOUNT_DEFAULT IN_TYPE(AMOUNT_DEFAULT_FLOAT)
#endif


#define IN_EVAL CONCAT2(eval,IN_TYPE)
#define AMOUNT_EVAL CONCAT2(eval,AMOUNT_TYPE)

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
    AttributeKey<AMOUNT_TYPE> amountAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    amountAttr = sceneClass.declareAttribute<AMOUNT_TYPE>("amount", AMOUNT_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
    mIspc.disableMode = get(disableAttr);
}

float
invert(float inValue, float amount)
{
    return amount - inValue;
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);
    const AMOUNT_TYPE amountValue = AMOUNT_EVAL(me, amountAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = invert(inValue, amountValue);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = invert(inValue.r, amountValue.r);
        outValue.g = invert(inValue.g, amountValue.g);
        outValue.b = invert(inValue.b, amountValue.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = invert(inValue.r, amountValue.r);
        outValue.g = invert(inValue.g, amountValue.g);
        outValue.b = invert(inValue.b, amountValue.b);
        outValue.a = invert(inValue.a, amountValue.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = invert(inValue.x, amountValue.x);
        outValue.y = invert(inValue.y, amountValue.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = invert(inValue.x, amountValue.x);
        outValue.y = invert(inValue.y, amountValue.y);
        outValue.z = invert(inValue.z, amountValue.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = invert(inValue.x, amountValue.x);
        outValue.y = invert(inValue.y, amountValue.y);
        outValue.z = invert(inValue.z, amountValue.z);
        outValue.w = invert(inValue.w, amountValue.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = invert(inValue.r, amountValue);
        outValue.g = invert(inValue.g, amountValue);
        outValue.b = invert(inValue.b, amountValue);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = invert(inValue.r, amountValue);
        outValue.g = invert(inValue.g, amountValue);
        outValue.b = invert(inValue.b, amountValue);
        outValue.a = invert(inValue.a, amountValue);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = invert(inValue.x, amountValue);
        outValue.y = invert(inValue.y, amountValue);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = invert(inValue.x, amountValue);
        outValue.y = invert(inValue.y, amountValue);
        outValue.z = invert(inValue.z, amountValue);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = invert(inValue.x, amountValue);
        outValue.y = invert(inValue.y, amountValue);
        outValue.z = invert(inValue.z, amountValue);
        outValue.w = invert(inValue.w, amountValue);
#endif
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(IN_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}

