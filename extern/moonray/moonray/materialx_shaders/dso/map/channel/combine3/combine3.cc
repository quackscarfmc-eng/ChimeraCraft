// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file combine3.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// color3, vector3

#define OPERATION combine3
#define IN_DEFAULT_FLOAT 0.0f

//
// Configure the variants
//
#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define EXTRACT_TYPE IN_TYPE
#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/MapApi.h>
#define RETURN_TYPE Color

//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<Float> in1Attr;
    AttributeKey<Float> in2Attr;
    AttributeKey<Float> in3Attr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    in1Attr = sceneClass.declareAttribute<Float>("in1", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in2Attr = sceneClass.declareAttribute<Float>("in2", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in3Attr = sceneClass.declareAttribute<Float>("in3", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const Float in1Value = evalFloat(me, in1Attr, tls, state);
    const Float in2Value = evalFloat(me, in2Attr, tls, state);
    const Float in3Value = evalFloat(me, in3Attr, tls, state);

    RETURN_TYPE outValue; // Should always be Color
    if (me->mIspc.disableMode) {
        outValue.r = 0.f;
        outValue.g = 0.f;
        outValue.b = 0.f;
    } else {
        outValue.r = in1Value;
        outValue.g = in2Value;
        outValue.b = in3Value;
    }

    *sample = outValue;
}
