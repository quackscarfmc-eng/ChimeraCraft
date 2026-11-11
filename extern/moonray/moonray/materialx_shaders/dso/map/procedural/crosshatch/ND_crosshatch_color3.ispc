// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_crosshatch_color3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_crosshatch_color3 {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_crosshatch_color3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_crosshatch_color3* uniform me = MAP_GET_ISPC_CPTR(ND_crosshatch_color3, map);

    // Get attributes
    varying Vec2f uvtiling  = evalUvtilingAttr (map, tls, state);
    varying Vec2f uvoffset  = evalUvoffsetAttr (map, tls, state);
    varying float thickness = evalThicknessAttr(map, tls, state);
    varying bool  staggered = evalStaggeredAttr(map, tls, state);
    varying Vec2f texcoord;
    if (me->texcoordUseDefault) {
        texcoord = getSt(state);
    } else {
        texcoord = evalTexcoordAttr(map, tls, state);
    }

    // Scale and offset crosshatch pattern so that 1 tile maps to [0,1]x[0,1] in uv space
    varying Vec2f uv = texcoord * uvtiling - uvoffset;

    // Stagger in u direction if v's integer part is odd
    if (staggered) {
        varying float vFloor = floor(uv.y);
        varying int vInt = (int)vFloor;
        if (vInt & 1) uv.x = uv.x + 0.5f;
    }

    // Remap to 45 degrees rotated coordinates with a tilesize of 1/sqrt(2)
    varying float x = uv.x + uv.y;
    varying float y = uv.x - uv.y;

    // Find fractional coords with a rotated tile centred on the square gap between the crosshatches
    varying float xFrac = x - floor(x);
    varying float yFrac = y - floor(y);

    // Determine axial distances from tile centre
    varying float ax = abs(xFrac - 0.5f);
    varying float ay = abs(yFrac - 0.5f);

    // Pick greatest distance and threshold it appropriately
    varying float maxDist = max(ax, ay);
    varying float threshold = 0.5f - thickness * 0.70710678f;
    varying float out = (maxDist > threshold) ? 1.0f : 0.0f;

    // Output result
    return Color_ctor(out);
}

DEFINE_MAP_SHADER(ND_crosshatch_color3, sample)

