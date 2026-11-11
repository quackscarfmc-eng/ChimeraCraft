// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file lin_displayp3_to_lin_rec709.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

#include <scene_rdl2/common/math/ColorSpace.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

#define OPERATION lin_displayp3_to_lin_rec709
#define IN_DEFAULT_FLOAT 0.0f

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

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<IN_TYPE> inAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map* self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, RETURN_TYPE *sample);
    ispc::SHADER_NAME mIspc;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass, const std::string& name) : Parent(sceneClass, name)
{
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

IN_TYPE doLinDisplayp3ToLinRec709(IN_TYPE inValue)
{
    // Linear Display P3 to Linear Rec.709 transformation matrix
    // From MaterialX
    const float m00 =  1.22493029f;
    const float m01 = -0.04205868f;
    const float m02 = -0.01964128f;
    const float m10 = -0.22492968f;
    const float m11 =  1.04205894f;
    const float m12 = -0.07864794f;
    const float m20 =  0.00000006f;
    const float m21 = -0.00000001f;
    const float m22 =  1.09828925f;

    IN_TYPE outValue = inValue;
    float r = inValue.r;
    float g = inValue.g;
    float b = inValue.b;

    outValue.r = m00 * r + m01 * g + m02 * b;
    outValue.g = m10 * r + m11 * g + m12 * b;
    outValue.b = m20 * r + m21 * g + m22 * b;

    return outValue;
}

void SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                         const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);
    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);

    IN_TYPE outValue = inValue;
    if (!me->mIspc.disableMode) {
        outValue = doLinDisplayp3ToLinRec709(inValue);
    }

    *sample = outValue;
}
