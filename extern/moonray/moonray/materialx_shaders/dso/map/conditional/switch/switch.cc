// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file switch.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants. These are the in1 types, followed by a letter to 
// specify the type for the "which" attribute. If there's no letter after the type, it assumes
// "which" has a float type. If there's an I, it's an int.
//      - color3
//      - color3I
//      - color4
//      - color4I
//      - float
//      - floatI
//      - vector2
//      - vector2I
//      - vector3
//      - vector3I
//      - vector4
//      - vector4I
//
// Use #define VARIANT before including this file

#define OPERATION switch

//
// Configure the variants
//
// First, handle the input type
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3I)
#define IN_TYPE Color
#define IN_DEFAULT Color(0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4I)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(0.0f,0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,floatI)
#define IN_TYPE Float
#define IN_DEFAULT 0.0f
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2I)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3I)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.0f,0.0f,0.0f)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4I)
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
// Handle the "which" type
//
#if STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,float) || \
    STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector4)
#define WHICH_TYPE Float
#define WHICH_DEFAULT 0.0f

#elif STRING_CMP(VARIANT,color3I) || STRING_CMP(VARIANT,color4I) || STRING_CMP(VARIANT,floatI) || \
      STRING_CMP(VARIANT,vector2I) || STRING_CMP(VARIANT,vector3I) || STRING_CMP(VARIANT,vector4I)
#define WHICH_TYPE Int
#define WHICH_DEFAULT 0

#else
#error VARIANT is set to an invalid value
#endif

#define WHICH_EVAL CONCAT2(eval,WHICH_TYPE)

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
    AttributeKey<IN_TYPE> in3Attr;
    AttributeKey<IN_TYPE> in4Attr;
    AttributeKey<IN_TYPE> in5Attr;
    AttributeKey<WHICH_TYPE> whichAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    in1Attr = sceneClass.declareAttribute<IN_TYPE>("in1", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in2Attr = sceneClass.declareAttribute<IN_TYPE>("in2", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in3Attr = sceneClass.declareAttribute<IN_TYPE>("in3", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in4Attr = sceneClass.declareAttribute<IN_TYPE>("in4", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    in5Attr = sceneClass.declareAttribute<IN_TYPE>("in5", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    whichAttr = sceneClass.declareAttribute<WHICH_TYPE>("which", WHICH_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass, const std::string& name) :
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
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    IN_TYPE in1Value = IN_EVAL(me, in1Attr, tls, state);
    IN_TYPE in2Value = IN_EVAL(me, in2Attr, tls, state);
    IN_TYPE in3Value = IN_EVAL(me, in3Attr, tls, state);
    IN_TYPE in4Value = IN_EVAL(me, in4Attr, tls, state);
    IN_TYPE in5Value = IN_EVAL(me, in5Attr, tls, state);
    WHICH_TYPE whichValue = WHICH_EVAL(me, whichAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = in1Value;
    } else {
        switch ((int)whichValue) {
            case 0: outValue = in1Value; break;
            case 1: outValue = in2Value; break;
            case 2: outValue = in3Value; break;
            case 3: outValue = in4Value; break;
            case 4: outValue = in5Value; break;
            default: outValue = in1Value; break;
        }
    }
    //
    // Covoert to the output type if necessary
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

