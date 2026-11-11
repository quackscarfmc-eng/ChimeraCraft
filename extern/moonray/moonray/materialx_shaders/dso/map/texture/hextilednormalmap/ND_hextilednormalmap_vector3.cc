// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file hextilednormalmap.cc

#include "attributes.cc"
#include "ND_hextilednormalmap_vector3_ispc_stubs.h"

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <materialx_shaders/map/MtlxHextile.h>
#include <moonray/common/mcrt_macros/moonray_static_check.h>
#include <moonray/rendering/shading/BasicTexture.h>
#include <moonray/rendering/shading/MapApi.h>


#include <scene_rdl2/render/util/stdmemory.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;


namespace {
// Ported from MaterialX glsl
// Blend 3 normals by blending the gradients
// Morten S. Mikkelsen, Surface Gradient–Based Bump Mapping Framework, Journal of
// Computer Graphics Techniques (JCGT), vol. 9, no. 3, 60–90, 2020
// http://jcgt.org/published/0009/03/04/
Vec3f
NormalsToGradient(const Vec3f& N, const Vec3f& Np)
{
    float d = dot(N, Np);
    const Vec3f g = (d * N - Np) / max(sEpsilon, scene_rdl2::math::abs(d));
    return g;
}

Vec3f
GradientBlend3Normals(const Vec3f& N,
                      const Vec3f& N1, float N1_weight,
                      const Vec3f& N2, float N2_weight,
                      const Vec3f& N3, float N3_weight)
{
    float w1 = clamp(N1_weight, 0.0f, 1.0f);
    float w2 = clamp(N2_weight, 0.0f, 1.0f);
    float w3 = clamp(N3_weight, 0.0f, 1.0f);

    const Vec3f g1 = NormalsToGradient(N, N1);
    const Vec3f g2 = NormalsToGradient(N, N2);
    const Vec3f g3 = NormalsToGradient(N, N3);

    // blend
    const Vec3f gg = w1 * g1 + w2 * g2 + w3 * g3;

    // gradient to normal
    return normalize(N - gg);
}

}

RDL2_DSO_CLASS_BEGIN(ND_hextilednormalmap_vector3, Map)

public:
    ND_hextilednormalmap_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map* self,
                             moonray::shading::TLState* tls,
                       const moonray::shading::State& state,
                             Color* sample);
    ispc::ND_hextilednormalmap_vector3 mIspc;

    std::unique_ptr<moonray::shading::BasicTexture> mTexture;

RDL2_DSO_CLASS_END(ND_hextilednormalmap_vector3)

ND_hextilednormalmap_vector3::ND_hextilednormalmap_vector3(
    const SceneClass& sceneClass,
    const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = (SampleFunc) ND_hextilednormalmap_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_hextilednormalmap_vector3_getSampleFunc();
    mOutputType = TYPE_RGB;

    mTexture = std::make_unique<moonray::shading::BasicTexture>(this, sLogEventRegistry);
}

void
ND_hextilednormalmap_vector3::update()
{
    mIspc.mDisableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
    mIspc.mTexCoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));

    mIspc.mNormalUseDefault = (!getBinding(normalAttr) && isDefault(normalAttr));
    mIspc.mTangentUseDefault = (!getBinding(tangentAttr) && isDefault(tangentAttr));
    mIspc.mBitangentUseDefault = (!getBinding(bitangentAttr) && isDefault(bitangentAttr));

    const scene_rdl2::rdl2::SceneVariables &sv = getSceneClass().getSceneContext()->getSceneVariables();
    mIspc.mFatalColor = asIspc(sv.get(scene_rdl2::rdl2::SceneVariables::sFatalColor));

    // default color handling, for image files that don't resolve
    Color defaultColor = get(defaultAttr);

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
ND_hextilednormalmap_vector3::sample(const Map* self,
                                           moonray::shading::TLState *tls,
                                     const moonray::shading::State& state,
                                           Color* sample)
{
    const ND_hextilednormalmap_vector3* me = static_cast<const ND_hextilednormalmap_vector3*>(self);

    const Vec3f stateN = state.getN();
    if (me->mIspc.mDisableMode) {
        *sample = Color(stateN.x, stateN.y, stateN.z);
        return;
    }

    if (me->mTexture == nullptr) {
        *sample = asCpp(me->mIspc.mFatalColor);
        return;
    }

    Vec2f texcoordValue;
    if (me->mIspc.mTexCoordUseDefault) {
        texcoordValue = state.getSt();
    } else {
        texcoordValue = evalVec2f(me, texcoordAttr, tls, state);
    }

    Vec3f N;
    if (me->mIspc.mNormalUseDefault) {
        N = normalize(stateN);
    } else {
        N = evalVec3f(me, normalAttr, tls, state);
    }

    Vec3f T;
    if (me->mIspc.mTangentUseDefault) {
        T = normalize(state.getdPds());
    } else {
        T = evalVec3f(me, tangentAttr, tls, state);
    }

    Vec3f B;
    if (me->mIspc.mBitangentUseDefault) {
        B = normalize(cross(T, N));
    } else {
        B = evalVec3f(me, bitangentAttr, tls, state);
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
    const Color4 tx0 = me->mTexture->sample(tls, state, tiledData.coords[0], d0);

    float d1[4];
    d1[0] = tiledData.ddx[1].x;
    d1[1] = tiledData.ddy[1].x;
    d1[2] = tiledData.ddx[1].y;
    d1[3] = tiledData.ddy[1].y;
    tiledData.coords[1].y = 1.0f - tiledData.coords[1].y;
    const Color4 tx1 = me->mTexture->sample(tls, state, tiledData.coords[1], d1);

    float d2[4];
    d2[0] = tiledData.ddx[2].x;
    d2[1] = tiledData.ddy[2].x;
    d2[2] = tiledData.ddx[2].y;
    d2[3] = tiledData.ddy[2].y;
    tiledData.coords[2].y = 1.0f - tiledData.coords[2].y;
    const Color4 tx2 = me->mTexture->sample(tls, state, tiledData.coords[2], d2);

    // normal map adjustments
    Vec3f nm0 = Vec3f(tx0.r, tx0.g, tx0.b);
    Vec3f nm1 = Vec3f(tx1.r, tx1.g, tx1.b);
    Vec3f nm2 = Vec3f(tx2.r, tx2.g, tx2.b);
    if (me->get(flip_gAttr)) {
        nm0.y = 1.0f - nm0.y;
        nm1.y = 1.0f - nm1.y;
        nm2.y = 1.0f - nm2.y;
    }
    nm0 = 2.0f * nm0 - Vec3f(1.0f);
    nm1 = 2.0f * nm1 - Vec3f(1.0f);
    nm2 = 2.0f * nm2 - Vec3f(1.0f);

    const float strength = evalFloat(me, strengthAttr, tls, state);

    // un-rotate vectors to match space for blending
    const Xform3f xr0 = Xform3f::rotate(N, tiledData.rotations[0]);
    const Xform3f xr1 = Xform3f::rotate(N, tiledData.rotations[1]);
    const Xform3f xr2 = Xform3f::rotate(N, tiledData.rotations[2]);
    const Vec3f t0 = transformVector(xr0, T) * strength;
    const Vec3f t1 = transformVector(xr1, T) * strength;
    const Vec3f t2 = transformVector(xr2, T) * strength;
    const Vec3f b0 = transformVector(xr0, B) * strength;
    const Vec3f b1 = transformVector(xr1, B) * strength;
    const Vec3f b2 = transformVector(xr2, B) * strength;

    const Vec3f n0 = normalize(t0 * nm0.x + b0 * nm0.y + N * nm0.z);
    const Vec3f n1 = normalize(t1 * nm1.x + b1 * nm1.y + N * nm1.z);
    const Vec3f n2 = normalize(t2 * nm2.x + b2 * nm2.y + N * nm2.z);

    const float falloff = evalFloat(me, falloffAttr, tls, state);
    const Vec3f w = moonray::shading::HextileComputeBlendWeights(Vec3f(1.0f), tiledData.weights, falloff);

    const Vec3f outNormal = GradientBlend3Normals(N,
                                                  n0, w.x,
                                                  n1, w.y,
                                                  n2, w.z);

    *sample = Color(outNormal.x, outNormal.y, outNormal.z);
}
