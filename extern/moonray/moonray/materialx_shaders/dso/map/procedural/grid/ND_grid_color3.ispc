// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_grid_color3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_grid_color3 {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_grid_color3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_grid_color3* uniform me = MAP_GET_ISPC_CPTR(ND_grid_color3, map);

    // Get attribute values
    varying Vec2f texcoord;
    if (me->texcoordUseDefault) {
        texcoord = getSt(state);
    } else {
        texcoord = evalTexcoordAttr(map, tls, state);
    }
    varying Vec2f uvtiling  = evalUvtilingAttr (map, tls, state);
    varying Vec2f uvoffset  = evalUvoffsetAttr (map, tls, state);
    varying float thickness = evalThicknessAttr(map, tls, state);
    varying bool  staggered = evalStaggeredAttr(map, tls, state);

    // Scale and offset crosshatch pattern so that 1 tile maps to [0,1]x[0,1] in uv space
    varying Vec2f uv = texcoord * uvtiling - uvoffset;

    // Stagger in u direction if v's integer part is odd
    if (staggered) {
        varying float vFloor = floor(uv.y);
        varying int vInt = (int)vFloor;
        if (vInt & 1) uv.x = uv.x + 0.5f;
    }

    // Find fractional coords
    varying float uFrac = uv.x - floor(uv.x);
    varying float vFrac = uv.y - floor(uv.y);

    // Determine axial distances from tile centre
    varying float au = abs(uFrac - 0.5f);
    varying float av = abs(vFrac - 0.5f);

    // Pick greatest distance and threshold it appropriately
    varying float maxDist = max(au, av);
    varying float threshold = 0.5f - 0.5f * thickness;
    varying float out = (maxDist > threshold) ? 1.0f : 0.0f;

    // Output result
    return Color_ctor(out);
}

DEFINE_MAP_SHADER(ND_grid_color3, sample)

