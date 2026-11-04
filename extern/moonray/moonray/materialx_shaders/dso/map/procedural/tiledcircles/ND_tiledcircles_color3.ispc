// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_tiledcircles_color3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_tiledcircles_color3 {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_tiledcircles_color3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_tiledcircles_color3* uniform me = MAP_GET_ISPC_CPTR(ND_tiledcircles_color3, map);

    // Get attribute values
    const varying Vec2f texcoord  = me->texcoordUseDefault ? (const varying Vec2f) getSt(state)
                                                           : evalTexcoordAttr(map, tls, state);
    const varying Vec2f uvtiling  = evalUvtilingAttr(map, tls, state);
    const varying Vec2f uvoffset  = evalUvoffsetAttr(map, tls, state);
    const varying float size      = evalSizeAttr(map, tls, state);
    const varying bool  staggered = evalStaggeredAttr(map, tls, state);

    // Scale and offset so that 1 rectangular tile maps to [0,1]x[0,1] in uv space
    const varying Vec2f uv = texcoord * uvtiling - uvoffset;
    varying Vec2f center;

    if (staggered) {
        // Use hexagonal lattice.
        // The following logic determines the center of the hexagonal cell the point (uv) falls into.
        // The hexagonal grid can be described using three sets of parallel lines, each set at 120 degrees to the
        // others. We project the point onto three axes (directions 0, 4, and 8), which are separated by 120 degrees.
        // The magic number 1.73205080757f is sqrt(3), which arises from the geometry of the hexagon.
        // The projections (d0, d4, d8) are scaled so that the distance between adjacent lines is 1.
        // Taking the floor of these projections gives us the indices of the nearest lines in each direction.
        // The combinations of these indices define the hexagonal cells.
        // Indices i=0,1,...,11 correspond to direction vectors rotated 30*i degrees counterclockwise from the x-axis.

        // Take (scaled) dot products with directions 0, 4, 8:
        const varying float d0 =  2.0f * uv.x;                      // dot uv with 2 * (1, 0)
        const varying float d4 = -uv.x + 1.73205080757f * uv.y;     // dot uv with 2 * (-1/2,  sqrt(3)/2)
        const varying float d8 = -uv.x - 1.73205080757f * uv.y;     // dot uv with 2 * (-1/2, -sqrt(3)/2)

        // Find floors corresponding to Voronoi region boundaries
        const varying float i0 = floor(d0);
        const varying float i4 = floor(d4);
        const varying float i8 = floor(d8);

        // The other indices (i2, i6, i10) are derived from the above to represent the other directions, in an
        // efficient way using less arithmetic. These relationships come from the symmetry of the hexagonal lattice.
        const varying float i2  = -i8 - 1.0f;   // = floor(d2)
        const varying float i6  = -i0 - 1.0f;   // = floor(d6)
        const varying float i10 = -i4 - 1.0f;   // = floor(d10)

        // Basis vectors for the lattice are prescaled by 1/sqrt(3) to reduce the number of multiplies needed.
        // These correspond to directions 3, 7, and 11 (which are (1,0) rotated by 90, 210, and 330 degrees).
        const varying Vec2f dir3  = Vec2f_ctor( 0.0f,  0.5773502769f);  // 1/sqrt(3) * (0, 1)
        const varying Vec2f dir7  = Vec2f_ctor(-0.5f, -0.2886751346f);  // 1/sqrt(3) * (-sqrt(3)/2, -1/2)
        const varying Vec2f dir11 = Vec2f_ctor( 0.5f, -0.2886751346f);  // 1/sqrt(3) * ( sqrt(3)/2, -1/2)

        // Find the center of the hexagonal cell containing the uv-point.
        // For each cell, exactly one of the three conditions will be true, due to the properties of the hexagonal grid.
        // The sum (i0 + i2), (i4 + i6), or (i8 + i10) modulo 3 selects the correct region.
        if ((int)(i0 + i2) % 3 == 0) {
            center = i0 * dir11 + i2 * dir3;
        } else if ((int)(i4 + i6) % 3 == 0) {
            center = i4 * dir3 + i6 * dir7;
        } else {
            center = i8 * dir7 + i10 * dir11;
        }

    } else {
        // Not staggered; use square lattice.
        // Simply find center of square containing the uv-point.
        center = Vec2f_ctor(floor(uv.x) + 0.5f, floor(uv.y) + 0.5f);
    }

    const varying float dSqr = lengthSqr(uv - center);
    const varying float rSqr = 0.25f * size * size;   // radius = 0.5 * size
    const varying float out = (dSqr < rSqr) ? 1.0f : 0.0f;

    // Output result
    return Color_ctor(out);
}

DEFINE_MAP_SHADER(ND_tiledcircles_color3, sample)

