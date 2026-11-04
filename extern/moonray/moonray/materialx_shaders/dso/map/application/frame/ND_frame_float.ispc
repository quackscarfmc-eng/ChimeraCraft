// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_frame_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/Shading.isph>
#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_frame_float
{
    uniform bool mDisableMode;
    uniform float mFrame;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_frame_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_frame_float* uniform me = MAP_GET_ISPC_CPTR(ND_frame_float, map);

    if (me->mDisableMode) {
        return Color_ctor(0.0f, 0.0f, 0.0f);
    }

    Color outValue = Color_ctor(me->mFrame, me->mFrame, me->mFrame);
    return outValue;
}

DEFINE_MAP_SHADER(ND_frame_float, sample)
