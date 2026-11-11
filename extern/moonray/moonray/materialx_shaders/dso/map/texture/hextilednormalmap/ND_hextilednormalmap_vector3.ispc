// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "hextilednormalmap.ispc"

#include "attributes.isph"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <materialx_shaders/map/ispc/MtlxHextile.isph>
#include <moonray/rendering/shading/ispc/BasicTexture.isph>
#include <moonray/rendering/shading/ispc/Shading.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>


struct ND_hextilednormalmap_vector3 {
    const uniform BASIC_TEXTURE_Data * uniform mTexture;
    uniform bool mDisableMode;
    uniform bool mTexCoordUseDefault;
    uniform bool mNormalUseDefault;
    uniform bool mTangentUseDefault;
    uniform bool mBitangentUseDefault;
    uniform Color mFatalColor;
    uniform Color mDefaultColor;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_hextilednormalmap_vector3);


Vec3f
NormalsToGradient(const varying Vec3f& N,
                  const varying Vec3f& Np)
{
    const varying float d = dot(N, Np);
    const varying Vec3f g = (d * N - Np) / max(sEpsilon, abs(d));
    return g;
}

Vec3f
GradientBlend3Normals(const varying Vec3f& N,
                      const varying Vec3f& N1, varying float N1_weight,
                      const varying Vec3f& N2, varying float N2_weight,
                      const varying Vec3f& N3, varying float N3_weight)
{
    const varying float w1 = clamp(N1_weight, 0.0f, 1.0f);
    const varying float w2 = clamp(N2_weight, 0.0f, 1.0f);
    const varying float w3 = clamp(N3_weight, 0.0f, 1.0f);

    const varying Vec3f g1 = NormalsToGradient(N, N1);
    const varying Vec3f g2 = NormalsToGradient(N, N2);
    const varying Vec3f g3 = NormalsToGradient(N, N3);

    // blend
    const Vec3f gg = w1 * g1 + w2 * g2 + w3 * g3;

    // gradient to normal
    return normalize(N - gg);
}


static varying Color
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform ND_hextilednormalmap_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_hextilednormalmap_vector3, map);

    const varying Vec3f stateN = getN(state);
    if (me->mDisableMode) {
        return Color_ctor(stateN.x, stateN.y, stateN.z);
    }

    if (!me->mTexture) {
        return me->mFatalColor;
    }

    varying Vec2f texcoordValue;
    if (me->mTexCoordUseDefault) {
        texcoordValue = getSt(state);
    } else {
        texcoordValue = evalVec2f(map, tls, state, texcoordAttr);
    }

    varying Vec3f N;
    if (me->mNormalUseDefault) {
        N = normalize(stateN);
    } else {
        N = evalVec3f(map, tls, state, normalAttr);
    }

    varying Vec3f T;
    if (me->mTangentUseDefault) {
        T = normalize(getdPds(state));
    } else {
        T = evalVec3f(map, tls, state, tangentAttr);
    }

    varying Vec3f B;
    if (me->mBitangentUseDefault) {
        B = normalize(cross(T, N));
    } else {
        B = evalVec3f(map, tls, state, bitangentAttr);
    }

    const varying Vec2f tiling = evalVec2f(map, tls, state, tilingAttr);

    // Generate 3 texture coordinates & respective derivatives on hexagonal grid
    varying HextileData tiledData;
    HextileCoord(texcoordValue * tiling,
                 evalFloat(map, tls, state, rotationAttr),
                 evalVec2f(map, tls, state, rotationrangeAttr),
                 evalFloat(map, tls, state, scaleAttr),
                 evalVec2f(map, tls, state, scalerangeAttr),
                 evalFloat(map, tls, state, offsetAttr),
                 evalVec2f(map, tls, state, offsetrangeAttr),
                 Vec2f_ctor(getdSdx(state), -getdTdx(state)),
                 Vec2f_ctor(getdSdy(state), -getdTdy(state)),
                 tiledData);

    // For each coordinate, sample the texture
    varying float d0[4] = {0.f, 0.f, 0.f, 0.f};
    d0[0] = tiledData.ddx[0].x;
    d0[1] = tiledData.ddy[0].x;
    d0[2] = tiledData.ddx[0].y;
    d0[3] = tiledData.ddy[0].y;
    tiledData.coords[0].y = 1.0f - tiledData.coords[0].y;
    const varying Col4f tx0 = BASIC_TEXTURE_sample(me->mTexture,
                                                    tls, state,
                                                    tiledData.coords[0],
                                                    d0);

    varying float d1[4] = {0.f, 0.f, 0.f, 0.f};
    d1[0] = tiledData.ddx[1].x;
    d1[1] = tiledData.ddy[1].x;
    d1[2] = tiledData.ddx[1].y;
    d1[3] = tiledData.ddy[1].y;
    tiledData.coords[1].y = 1.0f - tiledData.coords[1].y;
    const varying Col4f tx1 = BASIC_TEXTURE_sample(me->mTexture,
                                                    tls, state,
                                                    tiledData.coords[1],
                                                    d1);

    varying float d2[4] = {0.f, 0.f, 0.f, 0.f};
    d2[0] = tiledData.ddx[2].x;
    d2[1] = tiledData.ddy[2].x;
    d2[2] = tiledData.ddx[2].y;
    d2[3] = tiledData.ddy[2].y;
    tiledData.coords[2].y = 1.0f - tiledData.coords[2].y;
    const varying Col4f tx2 = BASIC_TEXTURE_sample(me->mTexture,
                                                    tls, state,
                                                    tiledData.coords[2],
                                                    d2);

    // normal map adjustments
    varying Vec3f nm0 = Vec3f_ctor(tx0.r, tx0.g, tx0.b);
    varying Vec3f nm1 = Vec3f_ctor(tx1.r, tx1.g, tx1.b);
    varying Vec3f nm2 = Vec3f_ctor(tx2.r, tx2.g, tx2.b);
    if (getFlip_gAttr(map)) {
        nm0.y = 1.0f - nm0.y;
        nm1.y = 1.0f - nm1.y;
        nm2.y = 1.0f - nm2.y;
    }
    nm0 = 2.0f * nm0 - Vec3f_ctor(1.0f);
    nm1 = 2.0f * nm1 - Vec3f_ctor(1.0f);
    nm2 = 2.0f * nm2 - Vec3f_ctor(1.0f);

    const varying float strength = evalFloat(map, tls, state, strengthAttr);

    // un-rotate vectors to match space for blending
    const varying Xform3f xr0 = Xform3f_rotate(N, tiledData.rotations.x);
    const varying Xform3f xr1 = Xform3f_rotate(N, tiledData.rotations.y);
    const varying Xform3f xr2 = Xform3f_rotate(N, tiledData.rotations.z);
    const varying Vec3f t0 = transformVector(xr0, T) * strength;
    const varying Vec3f t1 = transformVector(xr1, T) * strength;
    const varying Vec3f t2 = transformVector(xr2, T) * strength;
    const varying Vec3f b0 = transformVector(xr0, B) * strength;
    const varying Vec3f b1 = transformVector(xr1, B) * strength;
    const varying Vec3f b2 = transformVector(xr2, B) * strength;

    const varying Vec3f n0 = normalize(t0 * nm0.x + b0 * nm0.y + N * nm0.z);
    const varying Vec3f n1 = normalize(t1 * nm1.x + b1 * nm1.y + N * nm1.z);
    const varying Vec3f n2 = normalize(t2 * nm2.x + b2 * nm2.y + N * nm2.z);

    const varying float falloff = evalFloat(map, tls, state, falloffAttr);
    const varying Vec3f vOne = Vec3f_ctor(1.0f);
    const varying Vec3f w = HextileComputeBlendWeights(vOne, tiledData.weights, falloff);

    const varying Vec3f outNormal = GradientBlend3Normals(N,
                                                          n0, w.x,
                                                          n1, w.y,
                                                          n2, w.z);

    return Color_ctor(outNormal.x, outNormal.y, outNormal.z);
}


DEFINE_MAP_SHADER(ND_hextilednormalmap_vector3, sample)
