// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_ambientocclusion_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_ambientocclusion_float {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_ambientocclusion_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_ambientocclusion_float* uniform me = MAP_GET_ISPC_CPTR(ND_ambientocclusion_float, map);
    
    // Stub implementation
    return Color_ctor(1.0f, 1.0f, 1.0f);
}

DEFINE_MAP_SHADER(ND_ambientocclusion_float, sample)
