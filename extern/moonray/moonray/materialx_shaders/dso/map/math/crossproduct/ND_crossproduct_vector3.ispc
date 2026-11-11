// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_crossproduct_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_crossproduct_vector3 {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_crossproduct_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_crossproduct_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_crossproduct_vector3, map);

    varying Vec3f in1Value;
    in1Value = evalIn1Attr(map, tls, state);

    varying Vec3f in2Value;
    in2Value = evalIn2Attr(map, tls, state);

    varying Vec3f outValue;
    if (me->disableMode) {
        outValue = in1Value;
    } else {
        outValue = cross(in1Value, in2Value);
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_crossproduct_vector3, sample)

