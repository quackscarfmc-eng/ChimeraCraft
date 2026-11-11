// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file conditional.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants. These are the in1 and in2 types, followed by a letter to 
// specify the types of value 1 and value2. If there's no letter after the type, it assumes
// value1 and value2 have a float type. If there's a B, they are bool. If there's an I, they are int.
//      - color3
//      - color3B
//      - color3I
//      - color4
//      - color4B
//      - color4I
//      - float
//      - floatB
//      - floatI
//      - vector2
//      - vector2B
//      - vector2I
//      - vector3
//      - vector3B
//      - vector3I
//      - vector4
//      - vector4B
//      - vector4I
//
// It also supports the following operations:
//      - ifequal
//      - ifgreater
//      - ifgreatereq
// Use #define VARIANT and #define OPERATION before including this file

// OPERATIONS
// Since we want to use a different OPERATION value per file we need to
// define these here. This allows us to use the STRING_CMP macro.
// Usually, we would only need to define VARIANT values, which are already
// enumerated in MtlxVariantTypes.hh.
// (starting at 200 to avoid conflicting with MtlxVariantTypes)
#define UNIQUE_ID_ifequal               203
#define UNIQUE_ID_ifgreater             204
#define UNIQUE_ID_ifgreatereq           205

//
// Configure the variants
//
// First, handle the input type
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3B) || STRING_CMP(VARIANT,color3I)
#define IN_TYPE Color
#define IN_DEFAULT Color(0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4B) || STRING_CMP(VARIANT,color4I)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(0.0f,0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,floatB) || STRING_CMP(VARIANT,floatI)
#define IN_TYPE Float
#define IN_DEFAULT 0.0f
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2B) || STRING_CMP(VARIANT,vector2I)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3B) || STRING_CMP(VARIANT,vector3I)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4B) || STRING_CMP(VARIANT,vector4I)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(0.0f,0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_VEC4F

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

//
// Handle the value type
//
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector4)
#define VALUE_TYPE Float
#define VALUE_DEFAULT 0.0f

#elif STRING_CMP(VARIANT,color3I) || STRING_CMP(VARIANT,color4I) || STRING_CMP(VARIANT,floatI) || STRING_CMP(VARIANT,vector2I) || STRING_CMP(VARIANT,vector3I) || STRING_CMP(VARIANT,vector4I)
#define VALUE_TYPE Int
#define VALUE_DEFAULT 0

#elif STRING_CMP(VARIANT,color3B) || STRING_CMP(VARIANT,color4B) || STRING_CMP(VARIANT,floatB) || STRING_CMP(VARIANT,vector2B) || STRING_CMP(VARIANT,vector3B) || STRING_CMP(VARIANT,vector4B)
#define VALUE_TYPE Bool
#define VALUE_DEFAULT false
#else
#error VARIANT is set to an invalid value
#endif

#define VALUE_EVAL CONCAT2(eval,VALUE_TYPE)

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
    AttributeKey<IN_TYPE> in1Attr;
    AttributeKey<IN_TYPE> in2Attr;
    AttributeKey<VALUE_TYPE> value1Attr;
    AttributeKey<VALUE_TYPE> value2Attr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    in1Attr = sceneClass.declareAttribute<IN_TYPE>("in1", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in2Attr = sceneClass.declareAttribute<IN_TYPE>("in2", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    value1Attr = sceneClass.declareAttribute<VALUE_TYPE>("value1", VALUE_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    value2Attr = sceneClass.declareAttribute<VALUE_TYPE>("value2", VALUE_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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

void
SHADER_NAME::sample(const Map * self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    VALUE_TYPE value1;
    value1 = VALUE_EVAL(me, value1Attr, tls, state);

    VALUE_TYPE value2;
    value2 = VALUE_EVAL(me, value2Attr, tls, state);

    IN_TYPE in1Value;
    in1Value = IN_EVAL(me, in1Attr, tls, state);

    IN_TYPE in2Value;
    in2Value = IN_EVAL(me, in2Attr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = in1Value;
    } else {
#if STRING_CMP(OPERATION,ifequal)
        outValue = (value1 == value2) ? in1Value : in2Value;
#elif STRING_CMP(OPERATION,ifgreater)
        outValue = (value1 > value2) ? in1Value : in2Value;
#elif STRING_CMP(OPERATION,ifgreatereq)
        outValue = (value1 >= value2) ? in1Value : in2Value;
#elif defined(OPERATION)
#error OPERATION is set to an invalid value
#else
#error OPERATION isn't set
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

