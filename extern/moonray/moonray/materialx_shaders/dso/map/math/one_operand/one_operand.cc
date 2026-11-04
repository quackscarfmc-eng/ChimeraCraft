// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file one_operand.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following OPERATIONS with one input
// acos, asin, ceil, cos, exp, floor, ln, round, sign, sin, sqrt, tan
//
// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, matrix33, matrix44
// color3FA, vector2FA, vector3FA, vector4FA, matrix33FA, matrix44FA
//
// Not every combination of OPERATION and VARIANT is a standard MaterialX shader
// See README for list of shaders.

// OPERATIONS
// must define these for our STRING_CMP macro to work
#define UNIQUE_ID_absval 200
#define UNIQUE_ID_floor  201
#define UNIQUE_ID_ceil   202
#define UNIQUE_ID_round  203
#define UNIQUE_ID_sin    204
#define UNIQUE_ID_cos    205
#define UNIQUE_ID_tan    206
#define UNIQUE_ID_asin   207
#define UNIQUE_ID_acos   208
#define UNIQUE_ID_sqrt   209
#define UNIQUE_ID_ln     210
#define UNIQUE_ID_sign   211
#define UNIQUE_ID_exp    212

//
// Configure the variants
// The output type is always the same as IN
//
#if STRING_CMP(VARIANT,integer)
#define IN_TYPE Float
#define IN_DEFAULT 0
#define OUT_TYPE TYPE_INT

#elif STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_DEFAULT 0.0
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.0,0.0)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.0,0.0,0.0)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(0.0,0.0,0.0,0.0)
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_DEFAULT Color(0.0,0.0,0.0)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(0.0,0.0,0.0,0.0)
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,color3)
#define RETURN_TYPE Color
#else
#if STRING_CMP(VARIANT,integer)
#define RETURN_TYPE Int
#else
#define RETURN_TYPE IN_TYPE
#endif
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

float
doOperation(float in)
{
#if STRING_CMP(OPERATION,absval)
    return fabsf(in);
#elif STRING_CMP(OPERATION,floor)
    return floorf(in);
#elif STRING_CMP(OPERATION,ceil)
    return ceilf(in);
#elif STRING_CMP(OPERATION,round)
    return roundf(in);
#elif STRING_CMP(OPERATION,sin)
    return sinf(in);
#elif STRING_CMP(OPERATION,cos)
    return cosf(in);
#elif STRING_CMP(OPERATION,tan)
    return tanf(in);
#elif STRING_CMP(OPERATION,asin)
    return asinf(in);
#elif STRING_CMP(OPERATION,acos)
    return acosf(in);
#elif STRING_CMP(OPERATION,sqrt)
    return sqrtf(in);
#elif STRING_CMP(OPERATION,ln)
    return logf(in);
#elif STRING_CMP(OPERATION,exp)
    return expf(in);
#elif STRING_CMP(OPERATION,sign)
    if (in > 0.0) {
        return 1.0;
    } else if (in < 0.0) {
        return -1.0;
    } else {
        return 0.0;
    }
#elif defined(OPERATION)
#error OPERATION is set to an invalid value
#else
#error OPERATION isn't set
#endif
}

void
SHADER_NAME::sample(const Map * self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    IN_TYPE inValue;
    inValue = IN_EVAL(me, inAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = doOperation(inValue);
#elif STRING_CMP(VARIANT,integer)
        outValue = doOperation(inValue);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = doOperation(inValue.r);
        outValue.g = doOperation(inValue.g);
        outValue.b = doOperation(inValue.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = doOperation(inValue.r);
        outValue.g = doOperation(inValue.g);
        outValue.b = doOperation(inValue.b);
        outValue.a = doOperation(inValue.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = doOperation(inValue.x);
        outValue.y = doOperation(inValue.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = doOperation(inValue.x);
        outValue.y = doOperation(inValue.y);
        outValue.z = doOperation(inValue.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = doOperation(inValue.x);
        outValue.y = doOperation(inValue.y);
        outValue.z = doOperation(inValue.z);
        outValue.w = doOperation(inValue.w);
#endif
    }

    //
    // for Map shaders the output type needs to be copied into a Color for
    // the return value. For Map shaders (and for Color variant for map)
    // it can just copy the outValue.
    //
#if STRING_CMP(IN_TYPE,Float) && STRING_CMP(VARIANT,float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}

