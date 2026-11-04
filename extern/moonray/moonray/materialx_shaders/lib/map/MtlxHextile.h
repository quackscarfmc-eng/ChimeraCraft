// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "MtlxHextile.isph"

// Adapted from MtlX standard libs

#pragma once

#include <moonray/rendering/shading/MapApi.h>
#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec3.h>

namespace moonray {
namespace shading {


scene_rdl2::math::Vec3f
HextileComputeBlendWeights(const scene_rdl2::math::Vec3f &luminanceWeights,
                           const scene_rdl2::math::Vec3f &tileWeights,
                           float falloff);


struct HextileData
{
    scene_rdl2::math::Vec2f coords[3];
    scene_rdl2::math::Vec3f weights;
    scene_rdl2::math::Vec3f rotations;
    scene_rdl2::math::Vec2f ddx[3];
    scene_rdl2::math::Vec2f ddy[3];
};

// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, Journal of Computer Graphics
// Techniques (JCGT), vol. 11, no. 2, 77-94, 2022
// http://jcgt.org/published/0011/03/05/
HextileData
HextileCoord(
    const scene_rdl2::math::Vec2f& coord,
    float rotation,
    const scene_rdl2::math::Vec2f& rotationRange,
    float scale,
    const scene_rdl2::math::Vec2f& scaleRange,
    float offset,
    const scene_rdl2::math::Vec2f& offsetRange,
    const scene_rdl2::math::Vec2f& dCoordDx,
    const scene_rdl2::math::Vec2f& dCoordDy);


inline
scene_rdl2::math::Vec2f
lerp(const scene_rdl2::math::Vec2f& a,
     const scene_rdl2::math::Vec2f& b,
     const scene_rdl2::math::Vec2f& t)
{
    return scene_rdl2::math::Vec2f(scene_rdl2::math::lerp(a.x, b.x, t.x),
                                   scene_rdl2::math::lerp(a.y, b.y, t.y));
}

inline
scene_rdl2::math::Vec2f
mat2mult(const scene_rdl2::math::Vec2f& v,
         float m00, float m01, float m10, float m11)
{
    return scene_rdl2::math::Vec2f(m00 * v.x + m10 * v.y,
                                   m01 * v.x + m11 * v.y);
}

}
}