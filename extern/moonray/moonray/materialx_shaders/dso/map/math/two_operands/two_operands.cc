// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file two_operands.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;


// supports the following componentwise operations with two float arguments
// use #define OPERATION before including this file
// add, subtract, multiply, divide, modulo, atan2, power, safepower, min, max
//
// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, matrix33, matrix44
// color3FA, vector2FA, vector3FA, vector4FA, matrix33FA, matrix44FA
//
// Not every combination of OPERATION and VARIANT is a standard MaterialX shader
// See README for list of shaders.

// OPERATIONS
#define UNIQUE_ID_add       200
#define UNIQUE_ID_subtract  201
#define UNIQUE_ID_multiply  202
#define UNIQUE_ID_divide    203
#define UNIQUE_ID_modulo    204
#define UNIQUE_ID_atan2     205
#define UNIQUE_ID_power     206
#define UNIQUE_ID_safepower 207
#define UNIQUE_ID_min       208
#define UNIQUE_ID_max       209

// for all but atan2 the 0.0 is the default for in1
// for non-matrix types (and atan2 isn't used with
// matrix types)
#if STRING_CMP(OPERATION,atan2)
#define IN1_DEFAULT_FLOAT 1.0
#else
#define IN1_DEFAULT_FLOAT 0.0
#endif

// in2 is the identity for all operations but sometimes identity is 0.0
// and sometimes identity is 1.0
#if STRING_CMP(OPERATION,multiply) || STRING_CMP(OPERATION,divide) || \
    STRING_CMP(OPERATION,modulo) || STRING_CMP(OPERATION,power) || \
    STRING_CMP(OPERATION,safepower)
#define IN2_DEFAULT_FLOAT 1.0
#else
#define IN2_DEFAULT_FLOAT 0.0
#endif

//
// Configure the variants, starting with IN1
// Output type always matches IN1
//
#if STRING_CMP(VARIANT,float)

#define IN1_TYPE Float
#define IN1_DEFAULT IN1_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)

#define IN1_TYPE Vec2f
#define IN1_DEFAULT Vec2f(IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)

#define IN1_TYPE Vec3f
#define IN1_DEFAULT Vec3f(IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)

#define IN1_TYPE Vec4f
#define IN1_DEFAULT Vec4f(IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)

#define IN1_TYPE Color
#define IN1_DEFAULT Color(IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)

#define IN1_TYPE Rgba
#define IN1_DEFAULT Rgba(IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT,IN1_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,matrix33) || STRING_CMP(VARIANT,matrix33FA)

#define IN1_TYPE Mat3f
#define IN1_DEFAULT Mat3f(1.0,0.0,0.0, 0.0,1.0,0.0, 0.0,0.0,1.0)
#define OUT_TYPE TYPE_MAT3F

#elif STRING_CMP(VARIANT,matrix44) || STRING_CMP(VARIANT,matrix44FA)

#define IN1_TYPE Mat4f
#define IN1_DEFAULT Mat4f(1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0)
#define OUT_TYPE TYPE_MAT4F

#elif defined(VARIANT)
#error VARIANT isn't set or is set to an invalid value
#else
#error VARIANT isn't set or is set to an invalid value
#endif

// IN2 is either a float or matches IN1
#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA) || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA) || STRING_CMP(VARIANT,matrix33FA) || STRING_CMP(VARIANT,matrix44FA)
#define IN2_TYPE Float
#define IN2_ATTRKEY FloatAttrKeyISPC
#else
#define IN2_TYPE IN1_TYPE
#define IN2_ATTRKEY IN1_ATTRKEY
#endif

#define IN1_EVAL CONCAT2(eval,IN1_TYPE)
#define IN2_EVAL CONCAT2(eval,IN2_TYPE)

// Handle IN2 typed defaults
#if STRING_CMP(IN2_TYPE,Float)
#define IN2_DEFAULT IN2_DEFAULT_FLOAT

#elif STRING_CMP(IN2_TYPE,Vec2f)
#define IN2_DEFAULT Vec2f(IN2_DEFAULT_FLOAT, IN2_DEFAULT_FLOAT)

#elif STRING_CMP(IN2_TYPE,Vec3f)
#define IN2_DEFAULT Vec3f(IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT)

#elif STRING_CMP(IN2_TYPE,Vec4f)
#define IN2_DEFAULT Vec4f(IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT)

#elif STRING_CMP(IN2_TYPE,Color)
#define IN2_DEFAULT Color(IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT)

#elif STRING_CMP(IN2_TYPE,Rgba)
#define IN2_DEFAULT Rgba(IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT,IN2_DEFAULT_FLOAT)

#elif STRING_CMP(IN2_TYPE,Mat3f)
#define IN2_DEFAULT Mat3f(IN2_DEFAULT_FLOAT,0.0,0.0, 0.0,IN2_DEFAULT_FLOAT,0.0, 0.0,0.0,IN2_DEFAULT_FLOAT)

#elif STRING_CMP(IN2_TYPE,Mat4f)
#define IN2_DEFAULT Mat4f(IN2_DEFAULT_FLOAT,0.0,0.0,0.0, 0.0,IN2_DEFAULT_FLOAT,0.0,0.0, 0.0,0.0,IN2_DEFAULT_FLOAT,0.0, 0.0,0.0,0.0,IN2_DEFAULT_FLOAT)

#endif

#include <moonray/rendering/shading/MapApi.h>
#if STRING_CMP(IN1_TYPE,Float) || STRING_CMP(IN1_TYPE,Color) || STRING_CMP(IN1_TYPE,Vec2f) || STRING_CMP(IN1_TYPE,Vec3f)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN1_TYPE
#endif

//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<IN1_TYPE> in1Attr;
    AttributeKey<IN2_TYPE> in2Attr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    in1Attr = sceneClass.declareAttribute<IN1_TYPE>("in1", IN1_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in2Attr = sceneClass.declareAttribute<IN2_TYPE>("in2", IN2_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
doOperation(float in1, float in2)
{
#if STRING_CMP(OPERATION,add)
    return in1 + in2;
#elif STRING_CMP(OPERATION,subtract)
    return in1 - in2;
#elif STRING_CMP(OPERATION,multiply)
    return in1 * in2;
#elif STRING_CMP(OPERATION,divide)
    return in1 / in2;
#elif STRING_CMP(OPERATION,modulo)
    return fmod(in1, in2);
#elif STRING_CMP(OPERATION,atan2)
    return atan2f(in1, in2);
#elif STRING_CMP(OPERATION,power)
    return powf(in1, in2);
#elif STRING_CMP(OPERATION,safepower)
    // prevents trying to take roots of negative numbers
    return (in1 < 0.0) ? -powf(-in1,in2) : powf(in1,in2);
#elif STRING_CMP(OPERATION,min)
    return min(in1, in2);
#elif STRING_CMP(OPERATION,max)
    return max(in1, in2);
#elif defined(OPERATION)
#error OPERATION is set to an invalid value
#else
#error OPERATION isn't set
#endif
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    IN1_TYPE in1Value;
    in1Value = IN1_EVAL(me, in1Attr, tls, state);

    IN2_TYPE in2Value;
    in2Value = IN2_EVAL(me, in2Attr, tls, state);

    IN1_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = in1Value;
    } else {
#if STRING_CMP(VARIANT,float)
        outValue = doOperation(in1Value, in2Value);
#elif STRING_CMP(VARIANT,color3)
        outValue.r = doOperation(in1Value.r, in2Value.r);
        outValue.g = doOperation(in1Value.g, in2Value.g);
        outValue.b = doOperation(in1Value.b, in2Value.b);
#elif STRING_CMP(VARIANT,color4)
        outValue.r = doOperation(in1Value.r, in2Value.r);
        outValue.g = doOperation(in1Value.g, in2Value.g);
        outValue.b = doOperation(in1Value.b, in2Value.b);
        outValue.a = doOperation(in1Value.a, in2Value.a);
#elif STRING_CMP(VARIANT,vector2)
        outValue.x = doOperation(in1Value.x, in2Value.x);
        outValue.y = doOperation(in1Value.y, in2Value.y);
#elif STRING_CMP(VARIANT,vector3)
        outValue.x = doOperation(in1Value.x, in2Value.x);
        outValue.y = doOperation(in1Value.y, in2Value.y);
        outValue.z = doOperation(in1Value.z, in2Value.z);
#elif STRING_CMP(VARIANT,vector4)
        outValue.x = doOperation(in1Value.x, in2Value.x);
        outValue.y = doOperation(in1Value.y, in2Value.y);
        outValue.z = doOperation(in1Value.z, in2Value.z);
        outValue.w = doOperation(in1Value.w, in2Value.w);

//    
// matrix multiply and divide are special cases defined in the spec,
// not component-wise operations
//
#elif STRING_CMP(OPERATION,multiply) && (STRING_CMP(VARIANT,matrix33) || STRING_CMP(VARIANT,matrix44))
        outValue = in1Value * in2Value;
#elif STRING_CMP(OPERATION,divide) && (STRING_CMP(VARIANT,matrix33) || STRING_CMP(VARIANT,matrix44))
        outValue = in1Value / in2Value;

#elif STRING_CMP(VARIANT,matrix33)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value.vx.x);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value.vx.y);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value.vx.z);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value.vy.x);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value.vy.y);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value.vy.z);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value.vz.x);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value.vz.y);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value.vz.z);
#elif STRING_CMP(VARIANT,matrix44)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value.vx.x);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value.vx.y);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value.vx.z);
        outValue.vx.w = doOperation(in1Value.vx.w, in2Value.vx.w);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value.vy.x);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value.vy.y);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value.vy.z);
        outValue.vy.w = doOperation(in1Value.vy.w, in2Value.vy.w);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value.vz.x);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value.vz.y);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value.vz.z);
        outValue.vz.w = doOperation(in1Value.vz.w, in2Value.vz.w);
#elif STRING_CMP(VARIANT,color3FA)
        outValue.r = doOperation(in1Value.r, in2Value);
        outValue.g = doOperation(in1Value.g, in2Value);
        outValue.b = doOperation(in1Value.b, in2Value);
#elif STRING_CMP(VARIANT,color4FA)
        outValue.r = doOperation(in1Value.r, in2Value);
        outValue.g = doOperation(in1Value.g, in2Value);
        outValue.b = doOperation(in1Value.b, in2Value);
        outValue.a = doOperation(in1Value.a, in2Value);
#elif STRING_CMP(VARIANT,vector2FA)
        outValue.x = doOperation(in1Value.x, in2Value);
        outValue.y = doOperation(in1Value.y, in2Value);
#elif STRING_CMP(VARIANT,vector3FA)
        outValue.x = doOperation(in1Value.x, in2Value);
        outValue.y = doOperation(in1Value.y, in2Value);
        outValue.z = doOperation(in1Value.z, in2Value);
#elif STRING_CMP(VARIANT,vector4FA)
        outValue.x = doOperation(in1Value.x, in2Value);
        outValue.y = doOperation(in1Value.y, in2Value);
        outValue.z = doOperation(in1Value.z, in2Value);
        outValue.w = doOperation(in1Value.w, in2Value);
#elif STRING_CMP(VARIANT,matrix33FA)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value);
#elif STRING_CMP(VARIANT,matrix44FA)
        outValue.vx.x = doOperation(in1Value.vx.x, in2Value);
        outValue.vx.y = doOperation(in1Value.vx.y, in2Value);
        outValue.vx.z = doOperation(in1Value.vx.z, in2Value);
        outValue.vx.w = doOperation(in1Value.vx.w, in2Value);
        outValue.vy.x = doOperation(in1Value.vy.x, in2Value);
        outValue.vy.y = doOperation(in1Value.vy.y, in2Value);
        outValue.vy.z = doOperation(in1Value.vy.z, in2Value);
        outValue.vy.w = doOperation(in1Value.vy.w, in2Value);
        outValue.vz.x = doOperation(in1Value.vz.x, in2Value);
        outValue.vz.y = doOperation(in1Value.vz.y, in2Value);
        outValue.vz.z = doOperation(in1Value.vz.z, in2Value);
        outValue.vz.w = doOperation(in1Value.vz.w, in2Value);
#endif
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(IN1_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(IN1_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN1_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}

