// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file constant.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants. 
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// boolean, integer, matrix33, matrix44
//

#define OPERATION constant

//
// Configure the variants
// The output type is always the same as VALUE_TYPE
//
#if STRING_CMP(VARIANT,float)
#define VALUE_TYPE Float
#define VALUE_DEFAULT 0.0f
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2)
#define VALUE_TYPE Vec2f
#define VALUE_DEFAULT Vec2f(0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define VALUE_TYPE Vec3f
#define VALUE_DEFAULT Vec3f(0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)
#define VALUE_TYPE Vec4f
#define VALUE_DEFAULT Vec4f(0.0f,0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3)
#define VALUE_TYPE Color
#define VALUE_DEFAULT Color(0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define VALUE_TYPE Rgba
#define VALUE_DEFAULT Rgba(0.0f,0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,matrix33) 
#define VALUE_TYPE Mat3f
#define VALUE_DEFAULT Mat3f(1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f)
#define OUT_TYPE TYPE_MAT3F

#elif STRING_CMP(VARIANT,matrix44) 
#define VALUE_TYPE Mat4f
#define VALUE_DEFAULT Mat4f(1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f,1.0f)
#define OUT_TYPE TYPE_MAT4F

#elif STRING_CMP(VARIANT,boolean)
#define VALUE_TYPE Int
#define VALUE_DEFAULT 0
#define OUT_TYPE TYPE_INT

#elif STRING_CMP(VARIANT,integer)
#define VALUE_TYPE Int
#define VALUE_DEFAULT 0
#define OUT_TYPE TYPE_INT

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#if STRING_CMP(VALUE_TYPE,Bool)
#define VALUE_EVAL evalInt
#else
#define VALUE_EVAL CONCAT2(eval,VALUE_TYPE)
#endif

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(VALUE_TYPE,Float) || STRING_CMP(VALUE_TYPE,Vec2f) || STRING_CMP(VALUE_TYPE,Vec3f) || STRING_CMP(VALUE_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE VALUE_TYPE
#endif


//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<VALUE_TYPE> valueAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    valueAttr = sceneClass.declareAttribute<VALUE_TYPE>("value", VALUE_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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

void
SHADER_NAME::sample(const Map * self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const VALUE_TYPE inValue = VALUE_EVAL(me, valueAttr, tls, state);

    VALUE_TYPE outValue = inValue;
    if (me->mIspc.disableMode) {
        outValue = VALUE_DEFAULT;
    } 
    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(VALUE_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(VALUE_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(VALUE_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}

