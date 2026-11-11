// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_combine2_vector2.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_combine2_vector2 {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_combine2_vector2);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_combine2_vector2* uniform me = MAP_GET_ISPC_CPTR(ND_combine2_vector2, map);
    varying float in1Value = evalFloat(map, tls, state, in1Attr);
    varying float in2Value = evalFloat(map, tls, state, in2Attr);

    varying Vec2f outValue;
    if (me->disableMode) {
        outValue.x = 0.f;
        outValue.y = 0.f;
    } else {
        outValue.x = in1Value;
        outValue.y = in2Value;
    }
    return Color_ctor(outValue.x, outValue.y, 0.0f);
}

DEFINE_MAP_SHADER(ND_combine2_vector2, sample)
