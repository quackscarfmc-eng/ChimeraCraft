// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_checkerboard_color3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_checkerboard_color3 {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_checkerboard_color3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_checkerboard_color3* uniform me = MAP_GET_ISPC_CPTR(ND_checkerboard_color3, map);

    // Get attribute values
    varying Color color1Value   = evalColor1Attr  (map, tls, state);
    varying Color color2Value   = evalColor2Attr  (map, tls, state);
    varying Vec2f uvtilingValue = evalUvtilingAttr(map, tls, state);
    varying Vec2f uvoffsetValue = evalUvoffsetAttr(map, tls, state);
    varying Vec2f texcoordValue;
    if (me->texcoordUseDefault) {
        texcoordValue = getSt(state);
    } else {
        texcoordValue = evalTexcoordAttr(map, tls, state);
    }

    // Do procedural calculation
    varying Vec2f uv = texcoordValue * uvtilingValue - uvoffsetValue;
    varying int uvXor = (int)floor(uv.x) ^ (int)floor(uv.y);
    return (uvXor & 1) ? color1Value : color2Value;
}

DEFINE_MAP_SHADER(ND_checkerboard_color3, sample)

