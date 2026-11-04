// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_tiledhexagons_color3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_tiledhexagons_color3 {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_tiledhexagons_color3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_tiledhexagons_color3* uniform me = MAP_GET_ISPC_CPTR(ND_tiledhexagons_color3, map);

    // Get attribute values
    const varying Vec2f texcoord  = me->texcoordUseDefault ? (const varying Vec2f) getSt(state)
                                                          : evalTexcoordAttr(map, tls, state);
    const varying Vec2f uvtiling  = evalUvtilingAttr(map, tls, state);
    const varying Vec2f uvoffset  = evalUvoffsetAttr(map, tls, state);
    const varying float size      = evalSizeAttr(map, tls, state);
    const varying bool  staggered = evalStaggeredAttr(map, tls, state);

    // Scale and offset so that 1 rectangular tile maps to [0,1]x[0,1] in uv space
    const varying Vec2f uv = texcoord * uvtiling - uvoffset;

    varying float maxDistance;
    if (staggered) {
        // Use hexagonal lattice.
        // Indices i=0,1,...,11 correspond to direction vectors rotated 30*i degrees counterclockwise from the x-axis.
        // The following logic determines which hexagonal cell the point (uv) falls into.
        // The hexagonal grid can be described using three sets of parallel lines, each set at 120 degrees to the
        // others. We project the point onto three axes (directions 0, 4, and 8), which are separated by 120 degrees.
        // The projections (d0, d4, d8) are scaled so that the distance between adjacent lines is 1.
        // Taking the floor of these projections gives us the indices of the nearest lines in each direction.
        // The combinations of these indices define the hexagonal cells.

        // Take (scaled) dot products with directions 0, 4, 8:
        varying float d0 =  2.0f * uv.x;                    // dot uv with 2 * (1, 0)
        varying float d4 = -uv.x + 1.73205080757f * uv.y;   // dot uv with 2 * (-1/2,  sqrt(3)/2)
        varying float d8 = -uv.x - 1.73205080757f * uv.y;   // dot uv with 2 * (-1/2, -sqrt(3)/2)

        // Find floors corresponding to cell boundaries
        const varying float i0 = floor(d0);
        const varying float i4 = floor(d4);
        const varying float i8 = floor(d8);

        // Reduce dot products to their fractional values
        d0 -= i0;
        d4 -= i4;
        d8 -= i8;

        // Pick out the 120-degree pie slice containing the uv point and take the max of the 2 corresponding distances
        if ((int)(i0 - i8 - 1.0f) % 3 == 0) {
            maxDistance = max(d0, 1.0f - d8);
        } else if ((int)(i4 - i0 - 1.0f) % 3 == 0) {
            maxDistance = max(d4, 1.0f - d0);
        } else {
            maxDistance = max(d8, 1.0f - d4);
        }

    } else {
        // Not staggered; use square lattice.
        // Simply find center of square containing the uv-point.
        const varying Vec2f center = Vec2f_ctor(floor(uv.x) + 0.5f, floor(uv.y) + 0.5f);

        // Find position relative to center, and take (scaled) dot products with directions 0, 4, 8
        const varying Vec2f duv = uv - center;
        const varying float d0 =  2.0f * duv.x;
        const varying float d4 = -duv.x + 1.73205080757f * duv.y;
        const varying float d8 = -duv.x - 1.73205080757f * duv.y;

        // Take the max of the distances in all 6 directions
        const varying float m0 = max(d0, -d0);
        const varying float m4 = max(d4, -d4);
        const varying float m8 = max(d8, -d8);
        maxDistance = max(m0, max(m4, m8));
    }

    const varying float out = (maxDistance < size) ? 1.0f : 0.0f;

    // Output result
    return Color_ctor(out);
}

DEFINE_MAP_SHADER(ND_tiledhexagons_color3, sample)

