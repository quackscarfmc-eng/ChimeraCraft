// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_glossiness_anisotropy.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_glossiness_anisotropy {
    uniform bool mDisableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_glossiness_anisotropy);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_glossiness_anisotropy* uniform me = MAP_GET_ISPC_CPTR(ND_glossiness_anisotropy, map);

    const varying float gloss = evalGlossinessAttr(map, tls, state);
    const varying float roughness = 1.0f - saturate(gloss);
    const varying float perceptualRoughness = clamp(roughness * roughness, sEpsilon, 1.0f);

    const varying float anisotropy = evalAnisotropyAttr(map, tls, state);
    varying Vec2f outValue = Vec2f_ctor(roughness, roughness);
    if (me->mDisableMode) {
        outValue.x = 0.0f;
        outValue.y = 0.0f;
    } else if (anisotropy > 0.0f) {
        const varying float aspect = sqrt(1.0 - clamp(anisotropy, 0.0f, 0.98f));
        outValue.x = min(perceptualRoughness / aspect, 1.0f);
        outValue.y = perceptualRoughness * aspect;
    }

    return Color_ctor(outValue.x, outValue.y, 0.0f);
}

DEFINE_MAP_SHADER(ND_glossiness_anisotropy, sample)

