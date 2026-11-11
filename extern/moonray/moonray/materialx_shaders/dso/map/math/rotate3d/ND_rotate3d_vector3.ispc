// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_rotate3d_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_rotate3d_vector3 {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_rotate3d_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_rotate3d_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_rotate3d_vector3, map);
    const varying Vec3f inValue = evalInAttr(map, tls, state);

    varying Vec3f outValue;
    if (me->disableMode) {
        outValue = inValue;
    } else {
        const varying Vec3f axis = evalAxisAttr(map, tls, state);
        const varying float angle = deg2rad(evalAmountAttr(map, tls, state));
        const varying Mat4f mat = Mat4f_rotate(axis, angle);

        outValue = transformVector(mat, inValue);
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_rotate3d_vector3, sample)

