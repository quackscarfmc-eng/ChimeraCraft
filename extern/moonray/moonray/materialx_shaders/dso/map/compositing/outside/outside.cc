// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file outside.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <moonray/rendering/shading/MapApi.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

#define OPERATION outside

#define IN_DEFAULT_FLOAT 0.0


#if STRING_CMP(VARIANT,float)

#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color3)

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

#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Color)
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
    AttributeKey<Float> maskAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    maskAttr = sceneClass.declareAttribute<Float>("mask", 1.0, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END

//
// end of attributes.cc
//

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map* self,
                             moonray::shading::TLState* tls,
                       const moonray::shading::State& state,
                             RETURN_TYPE *sample);
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
SHADER_NAME::sample(const Map* self,
                          moonray::shading::TLState* tls,
                    const moonray::shading::State& state,
                          RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);
    const float maskValue = evalFloat(me, maskAttr, tls, state);

    const IN_TYPE outValue = me->mIspc.disableMode ? inValue : maskValue * inValue;

#if STRING_CMP(IN_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#else
    *sample = outValue;
#endif
}