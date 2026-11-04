// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_roughness_dual.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_roughness_dual {
    uniform bool mDisableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_roughness_dual);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_roughness_dual* uniform me = MAP_GET_ISPC_CPTR(ND_roughness_dual, map);
    const uniform bool disable = me->mDisableMode;

    const varying Vec2f roughness = evalRoughnessAttr(map, tls, state);
    const varying float x = disable ? 0.0f : clamp(roughness.x * roughness.x, sEpsilon, 1.0f);
    const varying float y = disable ? 0.0f : clamp(roughness.y * roughness.y, sEpsilon, 1.0f);

    return Color_ctor(x, y, 0.0f);
}

DEFINE_MAP_SHADER(ND_roughness_dual, sample)

