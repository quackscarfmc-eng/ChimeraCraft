// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_trianglewave_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_trianglewave_float {
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_trianglewave_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_trianglewave_float* uniform me = MAP_GET_ISPC_CPTR(ND_trianglewave_float, map);
    varying float inValue = evalInAttr(map, tls, state);
    varying float t = 2.0f * (inValue - floor(inValue));
    varying float out = (t < 1.0f) ? t : 2.0f-t;
    return Color_ctor(out);
}

DEFINE_MAP_SHADER(ND_trianglewave_float, sample)

