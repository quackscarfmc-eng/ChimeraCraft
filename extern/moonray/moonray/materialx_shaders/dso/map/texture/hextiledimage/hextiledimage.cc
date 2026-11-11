// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file hextiledimage.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <materialx_shaders/map/MtlxHextile.h>
#include <moonray/common/mcrt_macros/moonray_static_check.h>
#include <moonray/rendering/shading/BasicTexture.h>
#include <moonray/rendering/shading/MapApi.h>


#include <scene_rdl2/render/util/stdmemory.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants:
//  - color3
//  - color4

#define OPERATION hextiledimage
#define IN_DEFAULT_FLOAT 0.f


//
// Configure the variants

#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT, IN_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGBA

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define RETURN_TYPE IN_TYPE


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
    AttributeKey<Vec2f> texcoordAttr;
    AttributeKey<Float> falloffAttr;
    AttributeKey<Float> falloffcontrastAttr;
    AttributeKey<Float> offsetAttr;
    AttributeKey<Vec2f> offsetrangeAttr;
    AttributeKey<Float> rotationAttr;
    AttributeKey<Vec2f> rotationrangeAttr;
    AttributeKey<Float> scaleAttr;
    AttributeKey<Vec2f> scalerangeAttr;
    AttributeKey<Vec2f> tilingAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    defaultAttr = sceneClass.declareAttribute<IN_TYPE>("default", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    fileAttr = sceneClass.declareAttribute<String>("file", FLAGS_NONE, INTERFACE_GENERIC, {});
    texcoordAttr = sceneClass.declareAttribute<Vec2f>("texcoord", FLAGS_BINDABLE, INTERFACE_GENERIC, {});
        sceneClass.setMetadata(texcoordAttr, "label", "Texture Coordinates");
    falloffAttr = sceneClass.declareAttribute<Float>("falloff", 0.5f, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    falloffcontrastAttr = sceneClass.declareAttribute<Float>("falloffcontrast", 0.5f, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    offsetAttr = sceneClass.declareAttribute<Float>("offset", 1.0f, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    offsetrangeAttr = sceneClass.declareAttribute<Vec2f>("offsetrange", Vec2f(0.0f, 1.0f), FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    rotationAttr = sceneClass.declareAttribute<Float>("rotation", 1.0f, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    rotationrangeAttr = sceneClass.declareAttribute<Vec2f>("rotationrange", Vec2f(0.0f, 360.0f), FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    scaleAttr = sceneClass.declareAttribute<Float>("scale", 1.0f, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    scalerangeAttr = sceneClass.declareAttribute<Vec2f>("scalerange", Vec2f(0.5f, 2.0f), FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    tilingAttr = sceneClass.declareAttribute<Vec2f>("tiling", Vec2f(1.0f, 1.0f), FLAGS_BINDABLE, INTERFACE_GENERIC, {});
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

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(
    const SceneClass& sceneClass,
    const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;

    mTexture = std::make_unique<moonray::shading::BasicTexture>(this, sLogEventRegistry);
}

void
SHADER_NAME::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
    mIspc.mTexCoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));

    const scene_rdl2::rdl2::SceneVariables &sv = getSceneClass().getSceneContext()->getSceneVariables();
    mIspc.mFatalColor = asIspc(sv.get(scene_rdl2::rdl2::SceneVariables::sFatalColor));

    // default color handling, for image files that don't resolve
    IN_TYPE defaultValue = get(defaultAttr);
    Color defaultColor;
#if STRING_CMP(VARIANT,color3)
    defaultColor = defaultValue;
#elif STRING_CMP(VARIANT,color4)
    defaultColor = Color(defaultValue.r, defaultValue.g, defaultValue.b);
#endif

    mIspc.mDefaultColor = asIspc(defaultColor);

    moonray::shading::WrapType wrapS = moonray::shading::WrapType::Periodic;
    moonray::shading::WrapType wrapT = moonray::shading::WrapType::Periodic;

    // Update BasicTexture and make sure it is valid
    if (hasChanged(fileAttr)) {
        std::string errorStr;

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

void
SHADER_NAME::sample(const Map* self,
                          moonray::shading::TLState *tls,
                    const moonray::shading::State& state,
                          RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    if (me->mIspc.mDisableMode) {
        *sample = IN_DEFAULT;
        return;
    }

    if (me->mTexture == nullptr) {
#if STRING_CMP(VARIANT,color4)
        Color errColor = asCpp(me->mIspc.mFatalColor);
        *sample = Color4(errColor.r, errColor.g, errColor.b, 1.0f);
#else
        *sample = asCpp(me->mIspc.mFatalColor);
#endif
        return;
    }

    Vec2f texcoordValue;
    if (me->mIspc.mTexCoordUseDefault) {
        texcoordValue = state.getSt();
    } else {
        texcoordValue = evalVec2f(me, texcoordAttr, tls, state);
    }

    const Vec2f tiling = evalVec2f(me, tilingAttr, tls, state);

    // Generate 3 texture coordinates & respective derivatives on hexagonal grid
    moonray::shading::HextileData tiledData =
        moonray::shading::HextileCoord(texcoordValue * tiling,
                                       evalFloat(me, rotationAttr, tls, state),
                                       evalVec2f(me, rotationrangeAttr, tls, state),
                                       evalFloat(me, scaleAttr, tls, state),
                                       evalVec2f(me, scalerangeAttr, tls, state),
                                       evalFloat(me, offsetAttr, tls, state),
                                       evalVec2f(me, offsetrangeAttr, tls, state),
                                       Vec2f(state.getdSdx(), -state.getdTdx()),
                                       Vec2f(state.getdSdy(), -state.getdTdy()));

    // For each coordinate, sample the texture
    float d0[4];
    d0[0] = tiledData.ddx[0].x;
    d0[1] = tiledData.ddy[0].x;
    d0[2] = tiledData.ddx[0].y;
    d0[3] = tiledData.ddy[0].y;
    tiledData.coords[0].y = 1.0f - tiledData.coords[0].y;
    Color4 tx0 = me->mTexture->sample(tls, state, tiledData.coords[0], d0);
    Vec3f tx0v = Vec3f(tx0.r, tx0.g, tx0.b);

    float d1[4];
    d1[0] = tiledData.ddx[1].x;
    d1[1] = tiledData.ddy[1].x;
    d1[2] = tiledData.ddx[1].y;
    d1[3] = tiledData.ddy[1].y;
    tiledData.coords[1].y = 1.0f - tiledData.coords[1].y;
    Color4 tx1 = me->mTexture->sample(tls, state, tiledData.coords[1], d1);
    Vec3f tx1v = Vec3f(tx1.r, tx1.g, tx1.b);

    float d2[4];
    d2[0] = tiledData.ddx[2].x;
    d2[1] = tiledData.ddy[2].x;
    d2[2] = tiledData.ddx[2].y;
    d2[3] = tiledData.ddy[2].y;
    tiledData.coords[2].y = 1.0f - tiledData.coords[2].y;
    Color4 tx2 = me->mTexture->sample(tls, state, tiledData.coords[2], d2);
    Vec3f tx2v = Vec3f(tx2.r, tx2.g, tx2.b);

    const Vec3f lumacoeffs = Vec3f(0.2722287, 0.6740818, 0.0536895);
    Vec3f cw = Vec3f(dot(tx0v, lumacoeffs), dot(tx1v, lumacoeffs), dot(tx2v, lumacoeffs));
    float falloff_contrast = evalFloat(me, falloffcontrastAttr, tls, state);
    cw = lerp(Vec3f(1.0f), cw, falloff_contrast);

    float falloff = evalFloat(me, falloffAttr, tls, state);
    Vec3f w = moonray::shading::HextileComputeBlendWeights(cw, tiledData.weights, falloff);


    Color4 outColor = w.x * tx0 + w.y * tx1 + w.z * tx2;

    IN_TYPE outValue;
#if STRING_CMP(VARIANT,color3)
    outValue.r = outColor.r;
    outValue.g = outColor.g;
    outValue.b = outColor.b;
#elif STRING_CMP(VARIANT,color4)
    outColor.a = (tx0.a + tx1.a + tx2.a) / 3.0f;
    outValue = outColor;
#endif

    *sample = outValue;
}
