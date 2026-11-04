// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file contrast.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION contrast
#define IN_DEFAULT_FLOAT 0.0f
#define PIVOT_DEFAULT_FLOAT 0.5f
#define AMOUNT_DEFAULT_FLOAT 1.0f

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE  TYPE_RGB

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE  TYPE_RGB

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE  TYPE_RGB

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE  TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)
#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE  TYPE_RGB

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_TYPE  TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Handle pivot and amount contrast types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define CONTRAST_TYPE Float
#else
#define CONTRAST_TYPE IN_TYPE
#endif

#define CONTRAST_EVAL CONCAT2(eval,CONTRAST_TYPE)

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define CONTRAST_PIVOT_DEFAULT PIVOT_DEFAULT_FLOAT
#define CONTRAST_AMOUNT_DEFAULT AMOUNT_DEFAULT_FLOAT

#elif STRING_CMP(VARIANT,vector2)
#define CONTRAST_PIVOT_DEFAULT Vec2f(PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT)
#define CONTRAST_AMOUNT_DEFAULT Vec2f(AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector3)
#define CONTRAST_PIVOT_DEFAULT Vec3f(PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT)
#define CONTRAST_AMOUNT_DEFAULT Vec3f(AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector4)
#define CONTRAST_PIVOT_DEFAULT Vec4f(PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT)
#define CONTRAST_AMOUNT_DEFAULT Vec4f(AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color3)
#define CONTRAST_PIVOT_DEFAULT Color(PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT)
#define CONTRAST_AMOUNT_DEFAULT Color(AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color4)
#define CONTRAST_PIVOT_DEFAULT Rgba(PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT,PIVOT_DEFAULT_FLOAT)
#define CONTRAST_AMOUNT_DEFAULT Rgba(AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT,AMOUNT_DEFAULT_FLOAT)

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
    AttributeKey<CONTRAST_TYPE> pivotAttr;
    AttributeKey<CONTRAST_TYPE> amountAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    pivotAttr = sceneClass.declareAttribute<CONTRAST_TYPE>("pivot", CONTRAST_PIVOT_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    amountAttr = sceneClass.declareAttribute<CONTRAST_TYPE>("amount", CONTRAST_AMOUNT_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
    // it's a Map shader
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

float
contrast(float inValue,
         float pivot,
         float amount)
{
    // Early out: identity case
    if (isOne(amount)) return inValue;
    return (inValue - pivot) * max(0.0f, amount) + pivot;
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);
    const CONTRAST_TYPE pivot = CONTRAST_EVAL(me, pivotAttr, tls, state);
    const CONTRAST_TYPE amount = CONTRAST_EVAL(me, amountAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = contrast(inValue, pivot, amount);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = contrast(inValue.r, pivot.r, amount.r);
        outValue.g = contrast(inValue.g, pivot.g, amount.g);
        outValue.b = contrast(inValue.b, pivot.b, amount.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = contrast(inValue.r, pivot.r, amount.r);
        outValue.g = contrast(inValue.g, pivot.g, amount.g);
        outValue.b = contrast(inValue.b, pivot.b, amount.b);
        outValue.a = contrast(inValue.a, pivot.a, amount.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = contrast(inValue.x, pivot.x, amount.x);
        outValue.y = contrast(inValue.y, pivot.y, amount.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = contrast(inValue.x, pivot.x, amount.x);
        outValue.y = contrast(inValue.y, pivot.y, amount.y);
        outValue.z = contrast(inValue.z, pivot.z, amount.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = contrast(inValue.x, pivot.x, amount.x);
        outValue.y = contrast(inValue.y, pivot.y, amount.y);
        outValue.z = contrast(inValue.z, pivot.z, amount.z);
        outValue.w = contrast(inValue.w, pivot.w, amount.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = contrast(inValue.r, pivot, amount);
        outValue.g = contrast(inValue.g, pivot, amount);
        outValue.b = contrast(inValue.b, pivot, amount);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = contrast(inValue.r, pivot, amount);
        outValue.g = contrast(inValue.g, pivot, amount);
        outValue.b = contrast(inValue.b, pivot, amount);
        outValue.a = contrast(inValue.a, pivot, amount);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = contrast(inValue.x, pivot, amount);
        outValue.y = contrast(inValue.y, pivot, amount);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = contrast(inValue.x, pivot, amount);
        outValue.y = contrast(inValue.y, pivot, amount);
        outValue.z = contrast(inValue.z, pivot, amount);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = contrast(inValue.x, pivot, amount);
        outValue.y = contrast(inValue.y, pivot, amount);
        outValue.z = contrast(inValue.z, pivot, amount);
        outValue.w = contrast(inValue.w, pivot, amount);
#endif
    }

    //
    // Do conversion for output type if necessary.
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

