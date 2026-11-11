// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_place2d_vector2.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_place2d_vector2 {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_place2d_vector2);

enum OperationOrder {
    SRT = 0,
    TRS = 1
};

Vec2f
rotate2d(const Vec2f& uv, float r)
{
    const varying float theta = deg2rad(r);

    varying float sintheta, costheta;
    sincos(theta, &sintheta, &costheta);

    return Vec2f_ctor(uv.x * costheta - uv.y * sintheta,
                      uv.x * sintheta + uv.y * costheta);
}

Vec2f 
transform_SRT(const Vec2f& uv, const Vec2f& p, const Vec2f& s, float r, const Vec2f& o)
{
    // Performs -pivot, scale, rotate, translate, +pivot
    return rotate2d((uv - p) * s, r) + o + p;
}

Vec2f 
transform_TRS(const Vec2f& uv, const Vec2f& p, const Vec2f& s, float r, const Vec2f& o)
{
    // Performs -pivot, translate, rotate, scale, +pivot
    // which does not introduce texture shear
    return rotate2d(uv - p + o, r) * s + p;
}

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_place2d_vector2* uniform me = MAP_GET_ISPC_CPTR(ND_place2d_vector2, map);

    varying Vec2f texcoordValue = evalTexcoordAttr(map, tls, state);
    varying Vec2f pivotValue = evalPivotAttr(map, tls, state);
    varying Vec2f scaleValue = evalScaleAttr(map, tls, state);
    varying float rotateValue = evalRotateAttr(map, tls, state); // given in degrees
    varying Vec2f offsetValue = evalOffsetAttr(map, tls, state);
    varying int operationorderValue = evalOperationorderAttr(map, tls, state);

    varying Vec2f outValue;
    if (me->disableMode) {
        outValue = texcoordValue;
    } else {
        if (operationorderValue == SRT) {
            outValue = transform_SRT(texcoordValue, pivotValue, scaleValue, rotateValue, offsetValue);
        } else {
            outValue = transform_TRS(texcoordValue, pivotValue, scaleValue, rotateValue, offsetValue);
        }
    }
    return Color_ctor(outValue.x, outValue.y, 0.0f);
}

DEFINE_MAP_SHADER(ND_place2d_vector2, sample)

