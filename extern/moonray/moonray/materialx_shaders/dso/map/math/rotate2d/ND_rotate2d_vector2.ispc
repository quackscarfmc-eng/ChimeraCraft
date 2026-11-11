// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_rotate2d_vector2.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_rotate2d_vector2 {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_rotate2d_vector2);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_rotate2d_vector2* uniform me = MAP_GET_ISPC_CPTR(ND_rotate2d_vector2, map);
    const varying Vec2f inValue = evalInAttr(map, tls, state);

    varying Vec2f outValue;
    if (me->disableMode) {
        outValue = inValue;
    } else {
        const varying float amountValue = evalAmountAttr(map, tls, state);
        const varying float theta = deg2rad(amountValue);

        varying float sintheta, costheta;
        sincos(theta, &sintheta, &costheta);

        outValue = Vec2f_ctor(inValue.x * costheta - inValue.y * sintheta,
                              inValue.x * sintheta + inValue.y * costheta);
    }
    return Color_ctor(outValue.x, outValue.y, 0.0f);
}

DEFINE_MAP_SHADER(ND_rotate2d_vector2, sample)

