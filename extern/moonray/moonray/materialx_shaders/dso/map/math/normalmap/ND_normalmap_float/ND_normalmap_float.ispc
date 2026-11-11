// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_normalmap_float.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_normalmap_float {
    uniform bool mDisableMode;
    uniform bool mNormalUseDefault;
    uniform bool mTangentUseDefault;
    uniform bool mBitangentUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_normalmap_float);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_normalmap_float* uniform me = MAP_GET_ISPC_CPTR(ND_normalmap_float, map);

    varying Vec3f inValue = evalInAttr(map, tls, state);

    if (me->mDisableMode) {
        const varying Color outValue = Color_ctor(inValue.x, inValue.y, inValue.z);
        return outValue;
    }

    if (dot(inValue, inValue) < sEpsilon) inValue = Vec3f_ctor(0.5f, 0.5f, 1.f);

    const varying float scaleValue = evalScaleAttr(map, tls, state);

    varying Vec3f N;
    if (me->mNormalUseDefault) {
        N = normalize(getN(state));
    } else {
        N = evalNormalAttr(map, tls, state);
    }

    varying Vec3f T;
    if (me->mTangentUseDefault) {
        T = getdPds(state);
        T = normalize(T);
    } else {
        T = evalTangentAttr(map, tls, state);
    }

    varying Vec3f B;
    if (me->mBitangentUseDefault) {
        B = normalize(cross(T, N));
    } else {
        B = evalBitangentAttr(map, tls, state);
    }

    inValue = 2.f * inValue - Vec3f_ctor(1.f);
    varying Vec3f outValue = T * scaleValue * inValue.x +
                             B * scaleValue * inValue.y +
                             N * inValue.z;
    outValue = normalize(outValue);

    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_normalmap_float, sample)
