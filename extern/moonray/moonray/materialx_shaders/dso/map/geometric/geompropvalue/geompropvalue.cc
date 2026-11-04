// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file geompropvalue.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <moonray/rendering/shading/MapApi.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// bool, int, float, color3, color4, vector2, vector3, vector4, and string

#define OPERATION geompropvalue
#define DEFAULT_FLOAT 0.0f
#define DEFAULT_BOOL false
#define DEFAULT_INT 0
#define DEFAULT_STRING "" 

//
// Configure the variants
// The output type is always the same as DEFAULT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define DEFAULT_TYPE Float
#define DEFAULT_VALUE DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,boolean)
#define DEFAULT_TYPE Bool
#define DEFAULT_VALUE DEFAULT_BOOL
#define OUT_TYPE TYPE_BOOL

#elif STRING_CMP(VARIANT,integer)
#define DEFAULT_TYPE Int
#define DEFAULT_VALUE DEFAULT_INT
#define OUT_TYPE TYPE_INT

#elif STRING_CMP(VARIANT,string)
#define DEFAULT_TYPE String
#define DEFAULT_VALUE DEFAULT_STRING
#define OUT_TYPE TYPE_STRING

#elif STRING_CMP(VARIANT,vector2)
#define DEFAULT_TYPE Vec2f
#define DEFAULT_VALUE Vec2f(DEFAULT_FLOAT,DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define DEFAULT_TYPE Vec3f
#define DEFAULT_VALUE Vec3f(DEFAULT_FLOAT,DEFAULT_FLOAT,DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)
#define DEFAULT_TYPE Vec4f
#define DEFAULT_VALUE Vec4f(DEFAULT_FLOAT,DEFAULT_FLOAT,DEFAULT_FLOAT,DEFAULT_FLOAT)
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,color3)
#define DEFAULT_TYPE Color
#define DEFAULT_VALUE Color(DEFAULT_FLOAT,DEFAULT_FLOAT,DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define DEFAULT_TYPE Rgba
#define DEFAULT_VALUE Rgba(DEFAULT_FLOAT,DEFAULT_FLOAT,DEFAULT_FLOAT,DEFAULT_FLOAT)
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
#if STRING_CMP(DEFAULT_TYPE,Float) || STRING_CMP(DEFAULT_TYPE,Vec2f) || STRING_CMP(DEFAULT_TYPE,Vec3f) || STRING_CMP(DEFAULT_TYPE,Color)
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
    AttributeKey<Bool> disableAttr;
    AttributeKey<String> geompropAttr;
    AttributeKey<DEFAULT_TYPE> defaultAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    geompropAttr = sceneClass.declareAttribute<String>("geomprop", "", FLAGS_NONE, INTERFACE_GENERIC, {});
#if STRING_CMP(DEFAULT_TYPE,String) || STRING_CMP(DEFAULT_TYPE,Bool) || STRING_CMP(DEFAULT_TYPE,Integer)
    defaultAttr = sceneClass.declareAttribute<DEFAULT_TYPE>("default", DEFAULT_VALUE, FLAGS_NONE, INTERFACE_GENERIC, {});
#else
    defaultAttr = sceneClass.declareAttribute<DEFAULT_TYPE>("default", DEFAULT_VALUE, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
#endif
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

    void createMissingAttrLogEvent(const std::string& primAttrType,
                                   const std::string& primAttrName);

    void createUnsupportedStringLogEvent(const std::string& primAttrName);

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
SHADER_NAME::createMissingAttrLogEvent(const std::string& primAttrType,
                            const std::string& primAttrName)
{
    std::ostringstream os;
    os << "Missing primitive attribute '"
        << primAttrType << " " << primAttrName
        << "', using default value";

    mIspc.mMissingAttributeEvent = sLogEventRegistry.createEvent(scene_rdl2::logging::WARN_LEVEL,
                                                                 os.str());
}

void
SHADER_NAME::createUnsupportedStringLogEvent(const std::string& primAttrName)
{
    std::ostringstream os;
    os << "Primitive attribute '"
        << primAttrName
        << "' of type string is currently unsupported, using default value";

    mIspc.mUnsupportedStringEvent = sLogEventRegistry.createEvent(scene_rdl2::logging::WARN_LEVEL,
                                                                  os.str());
}

void
SHADER_NAME::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));

    if (hasChanged(geompropAttr)) {
        const std::string attrName = get(geompropAttr);
#if STRING_CMP(VARIANT,float)
        moonray::shading::TypedAttributeKey<float> attributeKey(attrName);
        createMissingAttrLogEvent("float", attrName);
#elif STRING_CMP(VARIANT,color3)
        moonray::shading::TypedAttributeKey<Rgb> attributeKey(attrName);
        createMissingAttrLogEvent("color3", attrName);
#elif STRING_CMP(VARIANT,color4)
        moonray::shading::TypedAttributeKey<Rgba> attributeKey(attrName);
        createMissingAttrLogEvent("color4", attrName);
#elif STRING_CMP(VARIANT,vector2)
        moonray::shading::TypedAttributeKey<Vec2f> attributeKey(attrName);
        createMissingAttrLogEvent("vector2", attrName);
#elif STRING_CMP(VARIANT,vector3)
        moonray::shading::TypedAttributeKey<Vec3f> attributeKey(attrName);
        createMissingAttrLogEvent("vector3", attrName);
#elif STRING_CMP(VARIANT,vector4)
        moonray::shading::TypedAttributeKey<Vec4f> attributeKey(attrName);
        createMissingAttrLogEvent("vector4", attrName);
#elif STRING_CMP(VARIANT,boolean)
        moonray::shading::TypedAttributeKey<Bool> attributeKey(attrName);
        createMissingAttrLogEvent("boolean", attrName);
#elif STRING_CMP(VARIANT,integer)
        moonray::shading::TypedAttributeKey<Int> attributeKey(attrName);
        createMissingAttrLogEvent("integer", attrName);
#elif STRING_CMP(VARIANT,string)
        moonray::shading::TypedAttributeKey<String> attributeKey(attrName);
        createUnsupportedStringLogEvent(attrName);
#endif
        mIspc.mPrimitiveAttributeIndex = attributeKey;
        mOptionalAttributes.push_back(attributeKey);
    }
}

void
SHADER_NAME::sample(const Map * self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    int key = me->mIspc.mPrimitiveAttributeIndex;

    DEFAULT_TYPE outValue;
    if (me->mIspc.mDisableMode) {
        outValue = DEFAULT_EVAL(me, defaultAttr, tls, state);
    } else {
        if (state.isProvided(key)) {
#if STRING_CMP(VARIANT,float)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<float>(key));
#elif STRING_CMP(VARIANT,color3)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Rgb>(key));
#elif STRING_CMP(VARIANT,color4)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Rgba>(key));
#elif STRING_CMP(VARIANT,vector2)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Vec2f>(key));
#elif STRING_CMP(VARIANT,vector3)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Vec3f>(key));
#elif STRING_CMP(VARIANT,vector4)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Vec4f>(key));
#elif STRING_CMP(VARIANT,boolean)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Bool>(key));
#elif STRING_CMP(VARIANT,integer)
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Int>(key));
#elif STRING_CMP(VARIANT,string)
            moonray::shading::logEvent(me, me->mIspc.mUnsupportedStringEvent);
            outValue = DEFAULT_EVAL(me, defaultAttr, tls, state);
#endif
        } else {
            moonray::shading::logEvent(me, me->mIspc.mMissingAttributeEvent);
            outValue = DEFAULT_EVAL(me, defaultAttr, tls, state);
        }
    }

    //
    // Convert to the putput type if necessary
    //
#if STRING_CMP(DEFAULT_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(DEFAULT_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(DEFAULT_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}

