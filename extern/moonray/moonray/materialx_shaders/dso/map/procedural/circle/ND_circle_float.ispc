// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_circle_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_circle_float {
    uniform bool texcoordUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_circle_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_circle_float* uniform me = MAP_GET_ISPC_CPTR(ND_circle_float, map);

    // Get attribute values
    varying Vec2f centerValue = evalCenterAttr(map, tls, state);
    varying float radiusValue = evalRadiusAttr(map, tls, state);
    varying Vec2f texcoordValue;
    if (me->texcoordUseDefault) {
        texcoordValue = getSt(state);
    } else {
        texcoordValue = evalTexcoordAttr(map, tls, state);
    }

    // Do procedural calculation
    varying float dSq = lengthSqr(texcoordValue - centerValue);
    varying float rSq = radiusValue * radiusValue;
    varying float outValue = (dSq < rSq) ? 1.0f : 0.0f;
    return Color_ctor(outValue);
}

DEFINE_MAP_SHADER(ND_circle_float, sample)

