// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file noise2d.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <moonray/common/noise/Perlin.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants. The "FA" version use a float as the second operand
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4, 
// color3FA, color4FA, vector2FA, vector3FA, vector4FA,
//

#define OPERATION noise2d
#define AMPLITUDE_DEFAULT_FLOAT 1.0f
#define PIVOT_DEFAULT_FLOAT 0.0f
#define TEXCOORD_DEFAULT_VEC2F Vec2f(0.0f, 0.0f)

//
// Configure the variants
// The output type is always the same as OUT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define OUT_TYPE Float
#define OUT_DEFAULT AMPLITUDE_DEFAULT_FLOAT
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2) || STRING_CMP(VARIANT,vector2FA)
#define OUT_TYPE Vec2f
#define OUT_DEFAULT Vec2f(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3) || STRING_CMP(VARIANT,vector3FA)
#define OUT_TYPE Vec3f
#define OUT_DEFAULT Vec3f(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4) || STRING_CMP(VARIANT,vector4FA)
#define OUT_TYPE Vec4f
#define OUT_DEFAULT Vec4f(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)
#define OUT_ATTRIBUTE_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3) || STRING_CMP(VARIANT,color3FA)
#define OUT_TYPE Color
#define OUT_DEFAULT Color(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4) || STRING_CMP(VARIANT,color4FA)
#define OUT_ATTRIBUTE_TYPE TYPE_RGBA
#define OUT_TYPE Rgba
#define OUT_DEFAULT Rgba(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define OUT_EVAL CONCAT2(eval,OUT_TYPE)

//
// Handle amplitude range types
//

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define AMPLITUDE_TYPE Float
#else
#define AMPLITUDE_TYPE OUT_TYPE
#endif

#define AMPLITUDE_EVAL CONCAT2(eval,AMPLITUDE_TYPE)

#if STRING_CMP(VARIANT,float) || STRING_CMP(VARIANT,vector2FA) || STRING_CMP(VARIANT,vector3FA) || STRING_CMP(VARIANT,vector4FA)\
    || STRING_CMP(VARIANT,color3FA) || STRING_CMP(VARIANT,color4FA)
#define AMPLITUDE_DEFAULT AMPLITUDE_DEFAULT_FLOAT

#elif STRING_CMP(VARIANT,vector2)
#define AMPLITUDE_DEFAULT Vec2f(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector3)
#define AMPLITUDE_DEFAULT Vec3f(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,vector4)
#define AMPLITUDE_DEFAULT Vec4f(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color3)
#define AMPLITUDE_DEFAULT Color(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)

#elif STRING_CMP(VARIANT,color4)
#define AMPLITUDE_DEFAULT Rgba(AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT,AMPLITUDE_DEFAULT_FLOAT)

#endif

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(OUT_TYPE,Float) || STRING_CMP(OUT_TYPE,Vec2f) || STRING_CMP(OUT_TYPE,Vec3f) || STRING_CMP(OUT_TYPE,Color)
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
    AttributeKey<AMPLITUDE_TYPE> amplitudeAttr;
    AttributeKey<Float> pivotAttr;
    AttributeKey<Vec2f> texcoordAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    amplitudeAttr = sceneClass.declareAttribute<AMPLITUDE_TYPE>("amplitude", AMPLITUDE_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    pivotAttr = sceneClass.declareAttribute<Float>("pivot", PIVOT_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    texcoordAttr = sceneClass.declareAttribute<Vec2f>("texcoord", TEXCOORD_DEFAULT_VEC2F, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END

//
// end of attributes.cc
//

//
// Shader class declaration
//
RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);

    void update();

private:
    static void sample(const Map* self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, RETURN_TYPE *sample);

    ispc::SHADER_NAME mIspc;
    std::unique_ptr<moonray::noise::Perlin> mNoise;

RDL2_DSO_CLASS_END(SHADER_NAME)

//
// Shader class implementation
//
SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
                         const std::string& name) :
    Parent(sceneClass, name)
{
    // the functions pointers are unions so it doesn't
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
    mNoise = std::make_unique<moonray::noise::Perlin>();
    mIspc.mNoise = mNoise->getIspcPerlin();
}

void
SHADER_NAME::sample(const Map* self,
                    moonray::shading::TLState *tls,
                    const moonray::shading::State& state,
                    RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const AMPLITUDE_TYPE amplitude = AMPLITUDE_EVAL(me, amplitudeAttr, tls, state);
    const Float pivot = evalFloat(me, pivotAttr, tls, state);
    const Vec2f texcoord2d = evalVec2f(me, texcoordAttr, tls, state);
    const Vec3f position(texcoord2d.x, texcoord2d.y, 0.0f);

    // random position offsets for channels
    const Vec3f Rx(37.0f,  53.0f,  0.0f);
    const Vec3f Gx(101.0f, 173.0f, 0.0f);
    const Vec3f Bx(307.0f, 449.0f, 0.0f);
    const Vec3f Ax(498.0f, 563.0f, 0.0f);

    OUT_TYPE outValue;

    const int maxLevel = 1;
    const float lacunarity = 2.0f;
    const float persistence = 0.5;



#if STRING_CMP(VARIANT,float)
    outValue = me->mNoise->perlinFractal3D(position, maxLevel, persistence, lacunarity) * amplitude + pivot;
#elif STRING_CMP(VARIANT,color3)
    outValue.r = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude.r + pivot;
    outValue.g = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude.g + pivot;
    outValue.b = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude.b + pivot;
#elif STRING_CMP(VARIANT,color4)
    outValue.r = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude.r + pivot;
    outValue.g = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude.g + pivot;
    outValue.b = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude.b + pivot;
    outValue.a = me->mNoise->perlinFractal3D(position + Ax, maxLevel, persistence, lacunarity) * amplitude.a + pivot;
#elif STRING_CMP(VARIANT,vector2)
    outValue.x = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude.x + pivot;
    outValue.y = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude.y + pivot;
#elif STRING_CMP(VARIANT,vector3)
    outValue.x = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude.x + pivot;
    outValue.y = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude.y + pivot;
    outValue.z = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude.z + pivot;
#elif STRING_CMP(VARIANT,vector4)
    outValue.x = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude.x + pivot;
    outValue.y = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude.y + pivot;
    outValue.z = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude.z + pivot;
    outValue.w = me->mNoise->perlinFractal3D(position + Ax, maxLevel, persistence, lacunarity) * amplitude.w + pivot;
#elif STRING_CMP(VARIANT,color3FA)
    outValue.r = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.g = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.b = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude + pivot;
#elif STRING_CMP(VARIANT,color4FA)
    outValue.r = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.g = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.b = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.a = me->mNoise->perlinFractal3D(position + Ax, maxLevel, persistence, lacunarity) * amplitude + pivot;
#elif STRING_CMP(VARIANT,vector2FA)
    outValue.x = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.y = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude + pivot;
#elif STRING_CMP(VARIANT,vector3FA)
    outValue.x = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.y = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.z = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude + pivot;
#elif STRING_CMP(VARIANT,vector4FA)
    outValue.x = me->mNoise->perlinFractal3D(position + Rx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.y = me->mNoise->perlinFractal3D(position + Gx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.z = me->mNoise->perlinFractal3D(position + Bx, maxLevel, persistence, lacunarity) * amplitude + pivot;
    outValue.w = me->mNoise->perlinFractal3D(position + Ax, maxLevel, persistence, lacunarity) * amplitude + pivot;
#endif

    // Convert to output type if necessary.
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
