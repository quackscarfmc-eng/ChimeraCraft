// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_hexagon_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_hexagon_float {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_hexagon_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_hexagon_float* uniform me = MAP_GET_ISPC_CPTR(ND_hexagon_float, map);

    // Get attribute values
    varying Vec2f texcoord;
    if (me->texcoordUseDefault) {
        texcoord = getSt(state);
    } else {
        texcoord = evalTexcoordAttr(map, tls, state);
    }
    varying Vec2f center = evalCenterAttr(map, tls, state);
    varying float radius = evalRadiusAttr(map, tls, state);

    // Do procedural calculation
    varying Vec2f uv = texcoord - center;
    varying float d0 = abs(uv.x);                               //   0, 180 degrees
    varying float d1 = abs(0.5f * uv.x + 0.8660254f * uv.y);    //  60, 240 degrees
    varying float d2 = abs(0.5f * uv.x - 0.8660254f * uv.y);    // 120, 300 degrees
    varying float d  = max(d0, max(d1, d2));
    varying float out = (d < radius) ? 1.0f : 0.0f;
    return Color_ctor(out);
}

DEFINE_MAP_SHADER(ND_hexagon_float, sample)

