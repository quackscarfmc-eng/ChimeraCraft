// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file g22_ap1_to_lin_rec709.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

#include <scene_rdl2/common/math/ColorSpace.h>
#include <scene_rdl2/common/math/Math.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION g22_ap1_to_lin_rec709
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
    // the functions pointers are unions so it doesn't
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
doG22Ap1ToLinRec709(IN_TYPE inValue)
{
    // Gamma 2.2 AP1 (ACEScg) to Linear Rec.709
    // Step 1: Apply inverse gamma of 2.2 to linearize
    IN_TYPE linearAp1 = inValue;
    linearAp1.r = max(0.0f, inValue.r);
    linearAp1.g = max(0.0f, inValue.g);
    linearAp1.b = max(0.0f, inValue.b);
    
    linearAp1.r = pow(linearAp1.r, 2.2f);
    linearAp1.g = pow(linearAp1.g, 2.2f);
    linearAp1.b = pow(linearAp1.b, 2.2f);
    
    // Step 2: Apply ACEScg (AP1) to Linear Rec.709 matrix
    const float m00 =  1.705050786f;
    const float m01 = -0.621792197f;
    const float m02 = -0.083258589f;
    const float m10 = -0.130256490f;
    const float m11 =  1.140804506f;
    const float m12 = -0.010548016f;
    const float m20 = -0.024003039f;
    const float m21 = -0.128968775f;
    const float m22 =  1.152971814f;

    IN_TYPE outValue = inValue;
    
    float r = linearAp1.r;
    float g = linearAp1.g;
    float b = linearAp1.b;

    outValue.r = m00 * r + m01 * g + m02 * b;
    outValue.g = m10 * r + m11 * g + m12 * b;
    outValue.b = m20 * r + m21 * g + m22 * b;

    return outValue;
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);

    IN_TYPE outValue = inValue;
    if (!me->mIspc.disableMode) {
        outValue = doG22Ap1ToLinRec709(inValue);
    }

    *sample = outValue;
}
