// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file tiledimage.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <moonray/common/mcrt_macros/moonray_static_check.h>
#include <moonray/rendering/shading/BasicTexture.h>
#include <moonray/rendering/shading/UdimTexture.h>
#include <moonray/rendering/shading/MapApi.h>

#include <scene_rdl2/render/util/stdmemory.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants:
//  - float
//  - color3
//  - color4
//  - vector2
//  - vector3
//  - vector4

#define OPERATION tiledimage
#define IN_DEFAULT_FLOAT 0.f


//
// Configure the variants

#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,vector2)
#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_VEC4F

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif


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
    AttributeKey<IN_TYPE> defaultAttr;
    AttributeKey<String> fileAttr;
    AttributeKey<Int> filtertypeAttr;
    AttributeKey<Int> frameendactionAttr;
    AttributeKey<Int> frameoffsetAttr;
    AttributeKey<String> framerangeAttr;
    AttributeKey<Vec2f> texcoordAttr;
    AttributeKey<Vec2f> uvoffsetAttr;
    AttributeKey<Vec2f> uvtilingAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    defaultAttr = sceneClass.declareAttribute<IN_TYPE>("default", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    fileAttr = sceneClass.declareAttribute<String>("file", FLAGS_NONE, INTERFACE_GENERIC, {});
    filtertypeAttr = sceneClass.declareAttribute<Int>("filtertype", 0, FLAGS_ENUMERABLE, INTERFACE_GENERIC, {});
        sceneClass.setEnumValue(filtertypeAttr, 0, "closest");
        sceneClass.setEnumValue(filtertypeAttr, 2, "cubic");
        sceneClass.setEnumValue(filtertypeAttr, 1, "linear");
        sceneClass.setMetadata(filtertypeAttr, "label", "Filter Type");
        sceneClass.setMetadata(filtertypeAttr, "comment", "NOT SUPPORTED BY MOONRAY");
    frameendactionAttr = sceneClass.declareAttribute<Int>("frameendaction", 0, FLAGS_ENUMERABLE, INTERFACE_GENERIC, {});
        sceneClass.setEnumValue(frameendactionAttr, 1, "clamp");
        sceneClass.setEnumValue(frameendactionAttr, 0, "constant");
        sceneClass.setEnumValue(frameendactionAttr, 3, "mirror");
        sceneClass.setEnumValue(frameendactionAttr, 2, "periodic");
        sceneClass.setMetadata(frameendactionAttr, "label", "Frame End Action");
    frameoffsetAttr = sceneClass.declareAttribute<Int>("frameoffset", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
        sceneClass.setMetadata(frameoffsetAttr, "label", "Frame Offset");
    framerangeAttr = sceneClass.declareAttribute<String>("framerange", FLAGS_NONE, INTERFACE_GENERIC, {});
        sceneClass.setMetadata(framerangeAttr, "label", "Frame Range");
    texcoordAttr = sceneClass.declareAttribute<Vec2f>("texcoord", FLAGS_BINDABLE, INTERFACE_GENERIC, {});
        sceneClass.setMetadata(texcoordAttr, "label", "Texture Coordinates");
    uvoffsetAttr = sceneClass.declareAttribute<Vec2f>("uvoffset", FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    uvtilingAttr = sceneClass.declareAttribute<Vec2f>("uvtiling", FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END


//
// end of attributes.cc
//
RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map* self,
                             moonray::shading::TLState* tls,
                       const moonray::shading::State& state,
                             RETURN_TYPE* sample);
    ispc::SHADER_NAME mIspc;

    std::unique_ptr<moonray::shading::BasicTexture> mTexture;
    std::unique_ptr<moonray::shading::UdimTexture> mUdimTexture;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(
    const SceneClass& sceneClass,
    const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
    mIspc.mTexCoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));

    // NYI
    String framerangeValue = get(framerangeAttr);
    Int frameoffsetValue = get(frameoffsetAttr);
    Int frameendactionValue = get(frameendactionAttr);

    const scene_rdl2::rdl2::SceneVariables &sv = getSceneClass().getSceneContext()->getSceneVariables();
    mIspc.mFatalColor = asIspc(sv.get(scene_rdl2::rdl2::SceneVariables::sFatalColor));

    // default color handling, for image files that don't resolve
    // TODO? specifying 4th channel of default
    IN_TYPE defaultValue = get(defaultAttr);
    Color defaultColor;
#if STRING_CMP(VARIANT,float)
    defaultColor = Color(defaultValue, defaultValue, defaultValue);
#elif STRING_CMP(VARIANT,color3)
    defaultColor = defaultValue;
#elif STRING_CMP(VARIANT,color4)
    defaultColor = Color(defaultValue.r, defaultValue.g, defaultValue.b);
#elif STRING_CMP(VARIANT,vector2)
    defaultColor = Color(defaultValue.x, defaultValue.y, 0.0f);
#elif STRING_CMP(VARIANT,vector3)
    defaultColor = Color(defaultValue.x, defaultValue.y, defaultValue.z);
#elif STRING_CMP(VARIANT,vector4)
    defaultColor = Color(defaultValue.x, defaultValue.y, defaultValue.z);
#endif

    mIspc.mDefaultColor = asIspc(defaultColor);

    moonray::shading::WrapType wrapS = moonray::shading::WrapType::Periodic;
    moonray::shading::WrapType wrapT = moonray::shading::WrapType::Periodic;

    // Update BasicTexture and make sure it is valid
    if (hasChanged(fileAttr)) {
        std::string errorStr;

        std::string filename = get(fileAttr);
        std::size_t udimPos = filename.find("<UDIM>");
        bool isUdimTexture = udimPos != std::string::npos;

        if (isUdimTexture) {
            mTexture = nullptr;
            mIspc.mTexture = nullptr;
            if (!mUdimTexture) {
                mUdimTexture = fauxstd::make_unique<moonray::shading::UdimTexture>(this);
                mIspc.mUdimTexture = &mUdimTexture->getUdimTextureData();
            }

            if (!mUdimTexture->update(this,
                                      sLogEventRegistry,
                                      filename,
                                      static_cast<ispc::TEXTURE_GammaMode>(3), // auto gamma correction
                                      wrapS,
                                      wrapT,
                                      true, // use default color
                                      asCpp(mIspc.mDefaultColor),
                                      asCpp(mIspc.mFatalColor),
                                      errorStr)) {
                fatal(getSceneClass().getName(), "(\"", getName() , "\"): ", errorStr);
                mUdimTexture = nullptr;
                mIspc.mUdimTexture = nullptr;
                return;
            }

            mIspc.mUdimTexture = &mUdimTexture->getUdimTextureData();

        } else {

            mUdimTexture = nullptr;
            mIspc.mUdimTexture = nullptr;
            if (!mTexture) {
                mTexture = fauxstd::make_unique<moonray::shading::BasicTexture>(this, sLogEventRegistry);
                mIspc.mTexture = &mTexture->getBasicTextureData();
            }

            if (!mTexture->update(get(fileAttr),
                                  static_cast<ispc::TEXTURE_GammaMode>(3), // auto gamma correction
                                  wrapS,
                                  wrapT,
                                  true, // use default color
                                  asCpp(mIspc.mDefaultColor),
                                  asCpp(mIspc.mFatalColor),
                                  errorStr)) {
                fatal(getSceneClass().getName(), "(\"", getName() , "\"): ", errorStr);
                mTexture = nullptr;
                mIspc.mTexture = nullptr;
                return;
            }

            mIspc.mTexture = &mTexture->getBasicTextureData();
        }


    }
}

void
SHADER_NAME::sample(const Map* self,
                          moonray::shading::TLState* tls,
                    const moonray::shading::State& state,
                          RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    if (me->mIspc.mDisableMode) {
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f)
        *sample = Color(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT);
#else
        *sample = IN_DEFAULT;
#endif
        return;
    }

    if (me->mTexture == nullptr && me->mUdimTexture == nullptr) {
# if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
        *sample = asCpp(me->mIspc.mFatalColor);
#else
        *sample = IN_DEFAULT;
#endif
        return;
    }

    Vec2f texcoordValue;
    if (me->mIspc.mTexCoordUseDefault) {
        texcoordValue = state.getSt();
    } else {
        texcoordValue = evalVec2f(me, texcoordAttr, tls, state);
    }

    float derivatives[4];
    derivatives[0] = state.getdSdx();
    derivatives[2] = state.getdSdy();
    derivatives[1] = -state.getdTdx();
    derivatives[3] = -state.getdTdy();

    const Vec2f tiling = evalVec2f(me, uvtilingAttr, tls, state);
    const Vec2f offset = evalVec2f(me, uvoffsetAttr, tls, state);
    Vec2f st = texcoordValue * tiling + offset;
    Color4 tx;

    if (me->mUdimTexture) {
        int udim = me->mUdimTexture->computeUdim(tls, st.x, st.y);
        if (udim == -1) {
# if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
        *sample = asCpp(me->mIspc.mFatalColor);
#else
        *sample = IN_DEFAULT;
#endif
            return;
        }
        st.x = st.x - int(st.x);
        st.y = st.y - int(st.y);
        // OIIO texture flip
        st.y = 1.f - st.y;

        tx = me->mUdimTexture->sample(tls,
                                      state,
                                      udim,
                                      st,
                                      derivatives);
    } else {
        tx = me->mTexture->sample(tls,
                                  state,
                                  st,
                                  derivatives);
    }


    IN_TYPE outValue;
#if STRING_CMP(VARIANT,float)
    outValue = tx[0];
#elif STRING_CMP(VARIANT,color3)
    outValue.r = tx[0];
    outValue.g = tx[1];
    outValue.b = tx[2];
#elif STRING_CMP(VARIANT,color4)
    outValue = tx;
#elif STRING_CMP(VARIANT,vector2)
    outValue.x = tx[0];
    outValue.y = tx[1];
#elif STRING_CMP(VARIANT,vector3)
    outValue.x = tx[0];
    outValue.y = tx[1];
    outValue.z = tx[2];
#elif STRING_CMP(VARIANT,vector4)
    outValue.x = tx[0];
    outValue.y = tx[1];
    outValue.z = tx[2];
    outValue.w = tx[3];
#endif

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
