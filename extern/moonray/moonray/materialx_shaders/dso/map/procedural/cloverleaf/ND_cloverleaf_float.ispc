// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_cloverleaf_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_cloverleaf_float {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_cloverleaf_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_cloverleaf_float* uniform me = MAP_GET_ISPC_CPTR(ND_cloverleaf_float, map);

    // Get attribute values
    varying Vec2f texcoord;
    if (me->texcoordUseDefault) {
        texcoord = getSt(state);
    } else {
        texcoord = evalTexcoordAttr(map, tls, state);
    }
    const varying Vec2f center = evalCenterAttr(map, tls, state);
    const varying float radius = evalRadiusAttr(map, tls, state);

    // Do procedural calculation
    // Note: we halve the value of center because the spec says it's "2x the coordinate of the center".
    // Also we use abs and max to exploit the 8-fold symmetry of the cloverleaf, and do some algebraic
    // reduction on the resulting test to improve efficiency.
    const varying Vec2f uv = abs(texcoord - 0.5f * center);
    return Color_ctor((lengthSqr(uv) < radius * max(uv.x, uv.y)) ? 1.0f : 0.0f);
}

DEFINE_MAP_SHADER(ND_cloverleaf_float, sample)

