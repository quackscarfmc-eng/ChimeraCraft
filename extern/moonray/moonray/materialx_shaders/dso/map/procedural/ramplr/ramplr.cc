// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ramplr.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// float, color3, color4, vector2, vector3, vector4
//

#define OPERATION ramplr
#define IN_DEFAULT_FLOAT 0.0f
#define OUT_DEFAULT_FLOAT 0.0f

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_DEFAULT OUT_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_DEFAULT Vec2f(OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_DEFAULT Vec3f(OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_DEFAULT Vec4f(OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_DEFAULT Color(OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define OUT_DEFAULT Rgba(OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT,OUT_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

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
    AttributeKey<Bool>    disableAttr;
    AttributeKey<Vec2f>   texcoordAttr;
    AttributeKey<IN_TYPE> valuelAttr;
    AttributeKey<IN_TYPE> valuerAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr  = sceneClass.declareAttribute<Bool>   ("disable",  0,           FLAGS_NONE,     INTERFACE_GENERIC, {});
    texcoordAttr = sceneClass.declareAttribute<Vec2f>  ("texcoord", Vec2f(0.0f), FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    valuelAttr   = sceneClass.declareAttribute<IN_TYPE>("valuel",   IN_DEFAULT,  FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    valuerAttr   = sceneClass.declareAttribute<IN_TYPE>("valuer",   IN_DEFAULT,  FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END


//
// end of attributes.cc
//

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map* self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, RETURN_TYPE *sample);
    ispc::SHADER_NAME mIspc;

RDL2_DSO_CLASS_END(SHADER_NAME)

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
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const IN_TYPE valuel = IN_EVAL(me, valuelAttr, tls, state);
    const IN_TYPE valuer = IN_EVAL(me, valuerAttr, tls, state);

    const Vec2f texcoord = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);
    const float u = texcoord.x;

    IN_TYPE out = me->mIspc.disableMode ? OUT_DEFAULT : lerp(valuel, valuer, u);

    //
    // Convert the output to the return type if needed. Otherwise just assign it.
    //
#if STRING_CMP(IN_TYPE,Float)
    *sample = Color(out, out, out);
#elif STRING_CMP(IN_TYPE,Vec2f)
    *sample = Color(out.x, out.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    *sample = Color(out.x, out.y, out.z);
#else
    *sample = out;
#endif
}

