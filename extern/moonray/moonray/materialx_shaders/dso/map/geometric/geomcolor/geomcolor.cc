// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file geomcolor.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <moonray/rendering/shading/MapApi.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// float, color3, color4

#define OPERATION geomcolor
#define DEFAULT_FLOAT 0.0f

//
// Configure the variants
// The output type is always the same as DEFAULT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define DEFAULT_TYPE Float
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color3)
#define DEFAULT_TYPE Color
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define DEFAULT_TYPE Rgba
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define DEFAULT_EVAL CONCAT2(eval,DEFAULT_TYPE)

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(DEFAULT_TYPE,Float) || STRING_CMP(DEFAULT_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE DEFAULT_TYPE
#endif


//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Int> indexAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    indexAttr = sceneClass.declareAttribute<Int>("index", 0, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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
    std::unique_ptr<moonray::shading::Xform> mXform;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
                         const std::string& name) :
    Parent(sceneClass, name)
{
    // the function pointers on Map are unions so it doesn't
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
    // Create TypedAttributeKey for "displayColor" attribute based on variant type
    const std::string attrName = "displayColor";
#if STRING_CMP(VARIANT,float)
    moonray::shading::TypedAttributeKey<float> attributeKey(attrName);
#elif STRING_CMP(VARIANT,color3)
    moonray::shading::TypedAttributeKey<Rgb> attributeKey(attrName);
#elif STRING_CMP(VARIANT,color4)
    moonray::shading::TypedAttributeKey<Rgba> attributeKey(attrName);
#endif
    mIspc.mPrimitiveAttributeIndex = attributeKey;
    mOptionalAttributes.push_back(attributeKey);
}

void
SHADER_NAME::sample(const Map * self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    int key = me->mIspc.mPrimitiveAttributeIndex;

    DEFAULT_TYPE outValue;
    
    if (state.isProvided(key)) {
#if STRING_CMP(VARIANT,float)
        outValue = state.getAttribute(moonray::shading::TypedAttributeKey<float>(key));
#elif STRING_CMP(VARIANT,color3)
        outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Rgb>(key));
#elif STRING_CMP(VARIANT,color4)
        outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Rgba>(key));
#endif
    } else {
        // Use default values when attribute is not provided
#if STRING_CMP(VARIANT,float)
        outValue = 0.0f;
#elif STRING_CMP(VARIANT,color3)
        outValue = Color(0.0f, 0.0f, 0.0f);
#elif STRING_CMP(VARIANT,color4)
        outValue = Rgba(0.0f, 0.0f, 0.0f, 1.0f);
#endif
    }

    //
    // for Map shaders the output type needs to be copied into a Color for
    // the return value. For Map shaders (and for Color variant for map)
    // it can just copy the out.
    //
#if STRING_CMP(DEFAULT_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#else
    *sample = outValue;
#endif
}
