// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file determinant.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants:
//  - matrix 3x3
//  - matrix 4x4

#define OPERATION determinant

//
// Configure the variants

#if STRING_CMP(VARIANT,matrix33)
#define IN_TYPE Mat3f
#define IN_DEFAULT Mat3f(1, 0, 0, 0, 1, 0, 0, 0, 1)

#elif STRING_CMP(VARIANT,matrix44)
#define IN_TYPE Mat4f
#define IN_DEFAULT Mat4f(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Map shader
//
#include <moonray/rendering/shading/MapApi.h>

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
                       const moonray::shading::State &state, Color *sample);
    ispc::SHADER_NAME mIspc;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = TYPE_RGB;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = get(disableAttr);
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);

    float outValue;
    if (me->mIspc.disableMode) {
        outValue = 1.0f;
    } else {
        outValue = inValue.det();
    }

    *sample = Color(outValue);
}

