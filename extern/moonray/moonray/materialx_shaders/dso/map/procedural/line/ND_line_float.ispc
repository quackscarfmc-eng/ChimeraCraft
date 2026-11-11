// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_line_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_line_float {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_line_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_line_float* uniform me = MAP_GET_ISPC_CPTR(ND_line_float, map);

    // Get attribute values
    varying Vec2f texcoord;
    if (me->texcoordUseDefault) {
        texcoord = getSt(state);
    } else {
        texcoord = evalTexcoordAttr(map, tls, state);
    }
    const varying Vec2f center = evalCenterAttr(map, tls, state);
    const varying Vec2f point1 = evalPoint1Attr(map, tls, state);
    const varying Vec2f point2 = evalPoint2Attr(map, tls, state);
    const varying float radius = evalRadiusAttr(map, tls, state);

    // The 1.39 spec says that center gets added to point1 and point2, but we can achieve the same result
    // more cheaply by subtracting it off texcoord
    const varying Vec2f uv = texcoord - center;

    const varying Vec2f mid = 0.5f * (point1 + point2);   // midpoint of line
    const varying Vec2f seg = 0.5f * (point2 - point1);   // segment from midpoint to point2
    const varying Vec2f pos = uv - mid;                   // position relative to midpoint

    const varying float pp = dot(pos, pos);               // dot products needed for coordinate transformation
    const varying float ps = dot(pos, seg);
    const varying float ss = dot(seg, seg);

    // Transform the point into the local coordinate frame of the line segment, with the origin at its midpoint and the
    // x-axis parallel to the line segment. For improved efficiency, the working coordinates are multiplied through by
    // length(seg), which allows for the removal of divide operations. Also, only the squares of some values are
    // generated, to avoid taking any square roots. Finally, the absolute value of x is used so we can be agnostic to
    // which endpoint we are comparing against, reducing the number of cases we must handle, and the xClamped value
    // effectively selects appropriately between the distance to the endpoint and the distance to the line segment.
    const varying float x = abs(ps);
    const varying float xClamped  = max(x - ss, 0.0f);
    const varying float y2 = pp * ss - ps * ps;
    const varying float r2 = radius * radius * ss;
    const varying float d2 = xClamped * xClamped + y2;

    return Color_ctor((r2 > d2) ? 1.0f : 0.0f);
}

DEFINE_MAP_SHADER(ND_line_float, sample)

