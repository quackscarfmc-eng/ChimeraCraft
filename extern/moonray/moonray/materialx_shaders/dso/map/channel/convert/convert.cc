// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file convert.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

#define OPERATION convert

// use #define VARIANT before including this file
//
// Configure the variants
//

#if STRING_CMP(VARIANT,float_color3)
#define IN_TYPE Float
#define IN_DEFAULT 0.0
#define OUT_TYPE Color
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,float_color4)
#define IN_TYPE Float
#define IN_DEFAULT 0.0
#define OUT_TYPE Rgba
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,float_vector2)
#define IN_TYPE Float
#define IN_DEFAULT 0.0
#define OUT_TYPE Vec2f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,float_vector3)
#define IN_TYPE Float
#define IN_DEFAULT 0.0
#define OUT_TYPE Vec3f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,float_vector4)
#define IN_TYPE Float
#define IN_DEFAULT 0.0
#define OUT_TYPE Vec4f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3_color4)
#define IN_TYPE Color
#define IN_DEFAULT Color(0.0,0.0,0.0)
#define OUT_TYPE Rgba
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,color3_vector2)
#define IN_TYPE Color
#define IN_DEFAULT Color(0.0,0.0,0.0)
#define OUT_TYPE Vec2f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color3_vector3)
#define IN_TYPE Color
#define IN_DEFAULT Color(0.0,0.0,0.0)
#define OUT_TYPE Vec3f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color3_vector4)
#define IN_TYPE Color
#define IN_DEFAULT Color(0.0,0.0,0.0)
#define OUT_TYPE Vec4f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color4_color3)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(0.0,0.0,0.0,1.0)
#define OUT_TYPE Color
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4_vector2)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(0.0,0.0,0.0,1.0)
#define OUT_TYPE Vec2f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4_vector3)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(0.0,0.0,0.0,1.0)
#define OUT_TYPE Vec3f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC3F

#elif STRING_CMP(VARIANT,color4_vector4)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(0.0,0.0,0.0,1.0)
#define OUT_TYPE Vec4f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,vector2_color3)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.0,0.0)
#define OUT_TYPE Color
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2_color4)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.0,0.0)
#define OUT_TYPE Rgba
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,vector2_vector3)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.0,0.0)
#define OUT_TYPE Vec3f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2_vector4)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.0,0.0)
#define OUT_TYPE Vec4f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,vector3_color3)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.0,0.0,0.0)
#define OUT_TYPE Color
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3_color4)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.0,0.0,0.0)
#define OUT_TYPE Rgba
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,vector3_vector2)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.0,0.0,0.0)
#define OUT_TYPE Vec2f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3_vector4)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.0,0.0,0.0)
#define OUT_TYPE Vec4f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,vector4_color3)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(0.0,0.0,0.0,1.0)
#define OUT_TYPE Color
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4_color4)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(0.0,0.0,0.0,1.0)
#define OUT_TYPE Rgba
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,vector4_vector2)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(0.0,0.0,0.0,1.0)
#define OUT_TYPE Vec2f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4_vector3)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(0.0,0.0,0.0,1.0)
#define OUT_TYPE Vec3f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,boolean_float)
#define IN_TYPE Bool
#define IN_DEFAULT 0
#define OUT_TYPE Float
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,boolean_color3)
#define IN_TYPE Bool
#define IN_DEFAULT 0
#define OUT_TYPE Color
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,boolean_color4)
#define IN_TYPE Bool
#define IN_DEFAULT 0
#define OUT_TYPE Rgba
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,boolean_vector2)
#define IN_TYPE Bool
#define IN_DEFAULT 0
#define OUT_TYPE Vec2f
#deine OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,boolean_vector3)
#define IN_TYPE Bool
#define IN_DEFAULT 0
#define OUT_TYPE Vec3f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,boolean_vector4)
#define IN_TYPE Bool
#define IN_DEFAULT 0
#define OUT_TYPE Vec4f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,integer_float)
#define IN_TYPE Int
#define IN_DEFAULT 0
#define OUT_TYPE Float
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,integer_boolean)
#define IN_TYPE Int
#define IN_DEFAULT 0
#define OUT_TYPE Bool
#define OUT_ATTRIBUTE_TYPE TYPE_BOOL

#elif STRING_CMP(VARIANT,integer_color3)
#define IN_TYPE Int
#define IN_DEFAULT 0
#define OUT_TYPE Color
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,integer_color4)
#define IN_TYPE Int
#define IN_DEFAULT 0
#define OUT_TYPE Rgba
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,integer_vector2)
#define IN_TYPE Int
#define IN_DEFAULT 0
#define OUT_TYPE Vec2f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,integer_vector3)
#define IN_TYPE Int
#define IN_DEFAULT 0
#define OUT_TYPE Vec3f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,integer_vector4)
#define IN_TYPE Int
#define IN_DEFAULT 0
#define OUT_TYPE Vec4f
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT is not set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(OUT_TYPE,Float) || STRING_CMP(OUT_TYPE,Color) || STRING_CMP(OUT_TYPE,Vec2f) || STRING_CMP(OUT_TYPE,Vec3f)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE OUT_TYPE
#endif

//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//
MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<IN_TYPE> inAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
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
    mOutputType = OUT_ATTRIBUTE_TYPE;
}

void
SHADER_NAME::update()
{
}


void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    IN_TYPE inValue;
    inValue = IN_EVAL(me, inAttr, tls, state);

    OUT_TYPE outValue;

#if STRING_CMP(VARIANT,float_color3)
    outValue = Color(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,float_color4)
    outValue = Rgba(inValue, inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,float_vector2)
    outValue = Vec2f(inValue, inValue);
#elif STRING_CMP(VARIANT,float_vector3)
    outValue = Vec3f(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,float_vector4)
    outValue = Vec4f(inValue, inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,color3_color4)
    outValue = Rgba(inValue.r, inValue.g, inValue.b, 1.0);
#elif STRING_CMP(VARIANT,color3_vector2)
    outValue = Vec2f(inValue.r, inValue.g);
#elif STRING_CMP(VARIANT,color3_vector3)
    outValue = Vec3f(inValue.r, inValue.g, inValue.b);
#elif STRING_CMP(VARIANT,color3_vector4)
    outValue = Vec4f(inValue.r, inValue.g, inValue.b, 1.0);
#elif STRING_CMP(VARIANT,color4_color3)
    outValue = Color(inValue.r, inValue.g, inValue.b);
#elif STRING_CMP(VARIANT,color4_vector2)
    outValue = Vec2f(inValue.r, inValue.g);
#elif STRING_CMP(VARIANT,color4_vector3)
    outValue = Vec3f(inValue.r, inValue.g, inValue.b);
#elif STRING_CMP(VARIANT,color4_vector4)
    outValue = Vec4f(inValue.r, inValue.g, inValue.b, inValue.a);
#elif STRING_CMP(VARIANT,vector2_color3)
    outValue = Color(inValue.x, inValue.y, 0.0);
#elif STRING_CMP(VARIANT,vector2_color4)
    outValue = Rgba(inValue.x, inValue.y, 0.0, 1.0);
#elif STRING_CMP(VARIANT,vector2_vector3)
    outValue = Vec3f(inValue.x, inValue.y, 0.0);
#elif STRING_CMP(VARIANT,vector2_vector4)
    outValue = Vec4f(inValue.x, inValue.y, 0.0, 1.0);
#elif STRING_CMP(VARIANT,vector3_color3)
    outValue = Color(inValue.x, inValue.y, inValue.z);
#elif STRING_CMP(VARIANT,vector3_color4)
    outValue = Rgba(inValue.x, inValue.y, inValue.z, 1.0);
#elif STRING_CMP(VARIANT,vector3_vector2)
    outValue = Vec2f(inValue.x, inValue.y);
#elif STRING_CMP(VARIANT,vector3_vector4)
    outValue = Vec4f(inValue.x, inValue.y, inValue.z, 1.0);
#elif STRING_CMP(VARIANT,vector4_color3)
    outValue = Color(inValue.x, inValue.y, inValue.z);
#elif STRING_CMP(VARIANT,vector4_color4)
    outValue = Rgba(inValue.x, inValue.y, inValue.z, inValue.w);
#elif STRING_CMP(VARIANT,vector4_vector2)
    outValue = Vec2f(inValue.x, inValue.y);
#elif STRING_CMP(VARIANT,vector4_vector3)
    outValue = Vec3f(inValue.x, inValue.y, inValue.z);
#elif STRING_CMP(VARIANT,boolean_float)
    outValue = inValue ? 1.0 : 0.0;
#elif STRING_CMP(VARIANT,boolean_color3)
    outValue = inValue ? Color(1.0, 1.0, 1.0) : Color(0.0, 0.0, 0.0);
#elif STRING_CMP(VARIANT,boolean_color4)
    outValue = inValue ? Rgba(1.0, 1.0, 1.0, 1.0) : Rgba(0.0, 0.0, 0.0, 1.0);
#elif STRING_CMP(VARIANT,boolean_vector2)
    outValue = inValue ? Vec2f(1.0, 1.0) : Vec2f(0.0, 0.0);
#elif STRING_CMP(VARIANT,boolean_vector3)
    outValue = inValue ? Vec3f(1.0, 1.0, 1.0) : Vec3f(0.0, 0.0, 0.0);
#elif STRING_CMP(VARIANT,boolean_vector4)
    outValue = inValue ? Vec4f(1.0, 1.0, 1.0, 1.0) : Vec4f(0.0, 0.0, 0.0, 1.0);
#elif STRING_CMP(VARIANT,boolean_integer)
    outValue = inValue ? 1 : 0;
#elif STRING_CMP(VARIANT,integer_float)
    outValue = inValue;
#elif STRING_CMP(VARIANT,integer_color3)
    outValue = Color(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,integer_color4)
    outValue = Rgba(inValue, inValue, inValue, 1.0);
#elif STRING_CMP(VARIANT,integer_vector2)
    outValue = Vec2f(inValue, inValue);
#elif STRING_CMP(VARIANT,integer_vector3)
    outValue = Vec3f(inValue, inValue, inValue);
#elif STRING_CMP(VARIANT,integer_vector4)
    outValue = Vec4f(inValue, inValue, inValue, 1.0);
#elif STRING_CMP(VARIANT,integer_boolean)
    outValue = inValue;
#endif

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(OUT_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(OUT_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(OUT_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}

