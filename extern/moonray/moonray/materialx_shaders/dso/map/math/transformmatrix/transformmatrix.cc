// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file transformmatrix.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants
//      - vector2M3
//      - vector3
//      - vector3M4
//      - vector4

#define OPERATION transformmatrix

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,vector2M3)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(0.f, 0.f)
#define MAT_TYPE Mat3f
#define MAT_DEFAULT Mat3f(1, 0, 0,   0, 1, 0,   0, 0, 1)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.f)
#define MAT_TYPE Mat3f
#define MAT_DEFAULT Mat3f(1, 0, 0,   0, 1, 0,   0, 0, 1)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3M4)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(0.f)
#define MAT_TYPE Mat4f
#define MAT_DEFAULT Mat4f(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(0.f)
#define MAT_TYPE Mat4f
#define MAT_DEFAULT Mat4f(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1)
#define OUT_TYPE TYPE_VEC4F

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)
#define MAT_EVAL CONCAT2(eval,MAT_TYPE)


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
    AttributeKey<IN_TYPE> inAttr;
    AttributeKey<MAT_TYPE> matAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    matAttr = sceneClass.declareAttribute<MAT_TYPE>("mat", MAT_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
    mSampleFunc = (SampleFunc)SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType =  OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = get(disableAttr);
}

Vec3f asVec3f(const Vec2f& v) { return Vec3f(v.x, v.y, 0.f); }
Vec2f asVec2f(const Vec3f& v) { return Vec2f(v.x, v.y); }
Vec4f asVec4f(const Vec3f& v) { return Vec4f(v.x, v.y, v.z, 0.f); }
Vec3f asVec3f(const Vec4f& v) { return Vec3f(v.x, v.y, v.z); }

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE inValue = IN_EVAL(me, inAttr, tls, state);
    const MAT_TYPE matValue = MAT_EVAL(me, matAttr, tls, state);

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,vector2M3)
        outValue = asVec2f(transformVector(matValue, asVec3f(inValue)));
#elif STRING_CMP(VARIANT,vector3)
        outValue = transformVector(matValue, inValue);
#elif STRING_CMP(VARIANT,vector3M4)
        outValue = transformVector(matValue, inValue);
#elif STRING_CMP(VARIANT,vector4)
        outValue = inValue * matValue;
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

