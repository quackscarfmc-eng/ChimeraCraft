// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_refract_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_refract_vector3 {
    uniform bool disableMode;
    uniform bool mNormalUseDefault;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_refract_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_refract_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_refract_vector3, map);

    varying Vec3f inValue;
    inValue = evalInAttr(map, tls, state);

    varying Vec3f normalValue;
    if (me->mNormalUseDefault) {
        normalValue = getN(state);
    } else {
        normalValue = evalNormalAttr(map, tls, state);
    }

    varying float iorValue;
    iorValue = evalIorAttr(map, tls, state);

    varying Vec3f outValue;
    if (me->disableMode) {
        outValue = inValue;
    } else {
        // Compute refraction using Snell's law
        // R = eta * I - (eta * cos(theta_i) + cos(theta_t)) * N
        // where eta = 1/ior (ratio of indices of refraction)
        // cos(theta_i) = -dot(N, I)
        // cos(theta_t) = sqrt(1 - eta^2 * (1 - cos(theta_i)^2))
        
        varying float eta = 1.0f / iorValue;
        varying float cosI = -dot(normalValue, inValue);
        varying float sinT2 = eta * eta * (1.0f - cosI * cosI);
        
        // Check for total internal reflection
        if (sinT2 > 1.0f) {
            // Total internal reflection - return reflection
            outValue = inValue - 2.0f * dot(inValue, normalValue) * normalValue;
        } else {
            varying float cosT = sqrt(1.0f - sinT2);
            outValue = eta * inValue + (eta * cosI - cosT) * normalValue;
        }
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_refract_vector3, sample)
