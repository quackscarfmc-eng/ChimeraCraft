// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_time_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/Shading.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_time_float
{
    uniform bool mDisableMode;
    uniform float mTime;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_time_float);

static Color
sample(const uniform Map* uniform map,
       uniform ShadingTLState* uniform tls,
       const varying State& state)
{
    const uniform ND_time_float* uniform me = MAP_GET_ISPC_CPTR(ND_time_float, map);

    if (me->mDisableMode) {
        return Color_ctor(0.f, 0.f, 0.f);
    }

    const float timeValue = me->mTime;
    return Color_ctor(timeValue, timeValue, timeValue);
}

DEFINE_MAP_SHADER(ND_time_float, sample)
