// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_combine2_color4CF.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_combine2_color4CF {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_combine2_color4CF);

static varying Col4f
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform ND_combine2_color4CF* uniform me = MAP_GET_ISPC_CPTR(ND_combine2_color4CF, map);
    varying Color in1Value = evalColor(map, tls, state, in1Attr);
    varying float in2Value = evalFloat(map, tls, state, in2Attr);

    varying Col4f outValue;
    if (me->disableMode) {
        outValue.r = 0.f;
        outValue.g = 0.f;
        outValue.b = 0.f;
        outValue.a = 0.f;
    } else {
        outValue.r = in1Value.r;
        outValue.g = in1Value.g;
        outValue.b = in1Value.b;
        outValue.a = in2Value;
    }
    return outValue;
}

DEFINE_MAP_SHADER_WITH_TYPE(ND_combine2_color4CF, sample, Col4f)
