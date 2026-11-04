// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_tiledcloverleafs_color3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_tiledcloverleafs_color3 {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_tiledcloverleafs_color3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_tiledcloverleafs_color3* uniform me = MAP_GET_ISPC_CPTR(ND_tiledcloverleafs_color3, map);

    // Get attribute values
    varying Vec2f texcoord;
    if (me->texcoordUseDefault) {
        texcoord = getSt(state);
    } else {
        texcoord = evalTexcoordAttr(map, tls, state);
    }
    const varying Vec2f uvtiling  = evalUvtilingAttr(map, tls, state);
    const varying Vec2f uvoffset  = evalUvoffsetAttr(map, tls, state);
    const varying float size      = evalSizeAttr(map, tls, state);
    const varying bool  staggered = evalStaggeredAttr(map, tls, state);

    // Scale and offset so that 1 rectangular tile maps to [0,1]x[0,1] in uv space
    const varying Vec2f uv = texcoord * uvtiling - uvoffset;

    // Determine center of nearest cloverleaf
    varying Vec2f center;
    if (staggered) {
        // For this shader, "staggered" means offsetting alternate rows by half a tile width while simultaneously
        // respacing the rows so that the resulting pattern is a square lattice rotated 45 degrees. Thus we
        // perform the rounding-to-nearest in a suitably rotated coordinate system.

        // Rotate, scale and offset
        const varying Vec2f uv1 = Vec2f_ctor(uv.x + uv.y + 0.5f, uv.x - uv.y + 0.5f);

        // Round
        const varying Vec2f uv2 = Vec2f_ctor(floor(uv1.x), floor(uv1.y));

        // Undo rotation and scale to give center
        center = 0.5f * Vec2f_ctor(uv2.x + uv2.y, uv2.x - uv2.y);
    } else {
        // Not staggered - simply find center of square containing the uv-point
        center = Vec2f_ctor(floor(uv.x) + 0.5f, floor(uv.y) + 0.5f);
    }

    // Do procedural calculation:
    // The cloverleaf shape is made of 4 overlapping disks - using abs() and max() picks the closest disk.
    const varying Vec2f duv = abs(uv - center);
    const varying float radius = 0.5f * size;
    const varying float out = (lengthSqr(duv) <= radius * max(duv.x, duv.y)) ? 1.0f : 0.0f;

    // Output result
    return Color_ctor(out);
}

DEFINE_MAP_SHADER(ND_tiledcloverleafs_color3, sample)

