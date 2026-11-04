// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_heighttonormal_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_heighttonormal_vector3 {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_heighttonormal_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_heighttonormal_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_heighttonormal_vector3, map);

    // Stub implementation: pass through normal
    varying Vec3f outValue = getN(state);

    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_heighttonormal_vector3, sample)
