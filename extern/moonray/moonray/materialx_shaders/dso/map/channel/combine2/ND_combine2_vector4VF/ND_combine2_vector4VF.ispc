// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_combine2_vector4VF.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_combine2_vector4VF {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_combine2_vector4VF);

static Vec4f
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform ND_combine2_vector4VF* uniform me = MAP_GET_ISPC_CPTR(ND_combine2_vector4VF, map);
    varying Vec3f in1Value = evalVec3f(map, tls, state, in1Attr);
    varying float in2Value = evalFloat(map, tls, state, in2Attr);

    varying Vec4f outValue;
    if (me->disableMode) {
        outValue.x = 0.f;
        outValue.y = 0.f;
        outValue.z = 0.f;
        outValue.w = 0.f;
    } else {
        outValue.x = in1Value.x;
        outValue.y = in1Value.y;
        outValue.z = in1Value.z;
        outValue.w = in2Value;
    }
    return outValue;
}

DEFINE_MAP_SHADER_WITH_TYPE(ND_combine2_vector4VF, sample, Vec4f)
