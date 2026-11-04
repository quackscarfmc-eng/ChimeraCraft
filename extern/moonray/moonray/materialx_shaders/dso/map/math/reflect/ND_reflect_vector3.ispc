// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_reflect_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_reflect_vector3 {
    uniform bool disableMode;
    uniform bool mNormalUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_reflect_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_reflect_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_reflect_vector3, map);

    varying Vec3f inValue;
    inValue = evalInAttr(map, tls, state);

    varying Vec3f normalValue;
    if (me->mNormalUseDefault) {
        normalValue = getN(state);
    } else {
        normalValue = evalNormalAttr(map, tls, state);
    }

    varying Vec3f outValue;
    if (me->disableMode) {
        outValue = inValue;
    } else {
        // Compute reflection: R = I - 2 * (I dot N) * N
        outValue = inValue - 2.0f * dot(inValue, normalValue) * normalValue;
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_reflect_vector3, sample)

