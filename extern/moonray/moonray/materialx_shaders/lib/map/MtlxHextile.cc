// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "MtlxHextile.cc"

#include "MtlxHextile.h"

using namespace scene_rdl2::math;

namespace moonray {
namespace shading {

// Christophe Schlick. “Fast Alternatives to Perlin’s Bias and Gain Functions”.
// In Graphics Gems IV, Morgan Kaufmann, 1994, pages 401–403.
// https://dept-info.labri.fr/~schlick/DOC/gem2.html
float
mxSchlickGain(float x, float r)
{
    float rr = clamp(r, 0.001f, 0.999f);
    float a = (1.0f / rr - 2.0f) * (1.0f - 2.0f * x);
    return (x < 0.5f) ? x / (a + 1.0f) : (a - x) / (a - 1.0f);
}

// Cheap hash without Sine, Dave Hoskins
// https://www.shadertoy.com/view/4djSRW
Vec2f
mxHextileHash(const Vec2f& p)
{
    Vec3f p3 = Vec3f(0.1031f * p.x,
                     0.1030f * p.y,
                     0.0973f * p.x);
    p3.x -= floor(p3.x);
    p3.y -= floor(p3.y);
    p3.z -= floor(p3.z);
    float f = dot(p3,
                  Vec3f(33.33f + p3.y,
                        33.33f + p3.z,
                        33.33f + p3.x));
    p3 = p3 + Vec3(f, f, f);
    Vec2f out;
    out.x = p3.x + p3.y;
    out.y = p3.x + p3.z;
    out.x *= p3.z;
    out.y *= p3.y;
    out.x -= floor(out.x);
    out.y -= floor(out.y);
    return out;
}


Vec3f
HextileComputeBlendWeights(const Vec3f& luminanceWeights,
                           const Vec3f& tileWeights,
                           float falloff)
{
    const Vec3f t3 = tileWeights * tileWeights * tileWeights;
    Vec3f w = luminanceWeights * tileWeights * t3 * t3;
    w /= (w.x + w.y + w.z);

    if (falloff != 0.5)
    {
        w.x = mxSchlickGain(w.x, falloff);
        w.y = mxSchlickGain(w.y, falloff);
        w.z = mxSchlickGain(w.z, falloff);
        w /= (w.x + w.y + w.z);
    }
    return w;
}


Vec2f
hexCenter(const Vec2f& v)
{
    /* skew using this matrix:
       1.0   0.5
       0.0   root(3)/2
    */
    const float r3_2 = 0.86602f;
    const float two_r3 = 3.4641f;
    Vec2f vf = v / two_r3;
    Vec2f out;
    out.x = vf.x + 0.5f * vf.y;
    out.y = r3_2 * vf.y;

    return out;
}

HextileData
HextileCoord(
    const Vec2f& coord,
    float rotation,
    const Vec2f& rotationRange,
    float scale,
    const Vec2f& scaleRange,
    float offset,
    const Vec2f& offsetRange,
    const Vec2f& dCoordDx,
    const Vec2f& dCoordDy)
{
    const float sqrt3_2 = sqrt(3.0f) * 2.0f;

    // scale coord to maintain the original fit
    Vec2f st = coord * sqrt3_2;

    // skew input space into simplex triangle grid
    // (1, 0, -tan(30), 2*tan(30))
    Vec2f stSkewed = mat2mult(st, 1.0, 0.0, -0.57735027f, 1.15470054f);

    // barycentric weights
    Vec2f stFrac;
    stFrac.x = stSkewed.x - floor(stSkewed.x);
    stFrac.y = stSkewed.y - floor(stSkewed.y);

    Vec3f temp = Vec3f(stFrac.x, stFrac.y, 0.0);
    temp.z = 1.0 - temp.x - temp.y;

    float s = -temp.z < 0.0 ? 0.0 : 1.0;
    float s2 = 2.0 * s - 1.0;

    float w1 = -temp.z * s2;
    float w2 = s - temp.y * s2;
    float w3 = s - temp.x * s2;

    // vertex IDs
    Vec2f baseID = Vec2f(floor(stSkewed.x), floor(stSkewed.y));

    Vec2f id1 = baseID + Vec2(s, s);
    Vec2f id2 = baseID + Vec2(s, 1.0f - s);
    Vec2f id3 = baseID + Vec2(1.0f - s, s);

    // tile center
    Vec2f ctr1 = mat2mult(id1 / Vec2f(sqrt3_2), 1.0f, 0.0f, 0.5f, 1.0f / 1.15470054f);
    Vec2f ctr2 = mat2mult(id2 / Vec2f(sqrt3_2), 1.0f, 0.0f, 0.5f, 1.0f / 1.15470054f);
    Vec2f ctr3 = mat2mult(id3 / Vec2f(sqrt3_2), 1.0f, 0.0f, 0.5f, 1.0f / 1.15470054f);

    // reuse hash for performance
    Vec2f seedOffset = Vec2f(0.12345);  // to avoid some zeros
    Vec2f rand1 = mxHextileHash(id1 + seedOffset);
    Vec2f rand2 = mxHextileHash(id2 + seedOffset);
    Vec2f rand3 = mxHextileHash(id3 + seedOffset);

    // randomized rotation matrix
    float rr_x = rotationRange.x * sPi / 180.0f;
    float rr_y = rotationRange.y * sPi / 180.0f;
    Vec3f rand_x = Vec3f(rand1.x, rand2.x, rand3.x);
    Vec3f rotations = lerp(Vec3f(rr_x), Vec3f(rr_y), rand_x * rotation);
    float sin_r_x = sin(rotations.x);
    float sin_r_y = sin(rotations.y);
    float sin_r_z = sin(rotations.z);
    float cos_r_x = cos(rotations.x);
    float cos_r_y = cos(rotations.y);
    float cos_r_z = cos(rotations.z);

    // randomized scale
    Vec3f rand_y = Vec3f(rand1.y, rand2.y, rand3.y);
    Vec3f scales = lerp(Vec3f(1.0f),
                        lerp(Vec3f(scaleRange.x),
                             Vec3f(scaleRange.y),
                             rand_y),
                        scale);
    Vec2f scale1 = Vec2f(scales.x);
    Vec2f scale2 = Vec2f(scales.y);
    Vec2f scale3 = Vec2f(scales.z);

    // randomized offset
    Vec2f offset1 = lerp(Vec2f(offsetRange.x), Vec2f(offsetRange.y), rand1 * offset);
    Vec2f offset2 = lerp(Vec2f(offsetRange.x), Vec2f(offsetRange.y), rand2 * offset);
    Vec2f offset3 = lerp(Vec2f(offsetRange.x), Vec2f(offsetRange.y), rand3 * offset);

    HextileData tile_data;
    tile_data.weights = Vec3f(w1, w2, w3);
    tile_data.rotations = rotations;

    // get coord
    tile_data.coords[0] = mat2mult(coord - ctr1, cos_r_x, -sin_r_x, sin_r_x, cos_r_x) / scale1 + ctr1 + offset1;
    tile_data.coords[1] = mat2mult(coord - ctr2, cos_r_y, -sin_r_y, sin_r_y, cos_r_y) / scale2 + ctr2 + offset2;
    tile_data.coords[2] = mat2mult(coord - ctr3, cos_r_z, -sin_r_z, sin_r_z, cos_r_z) / scale3 + ctr3 + offset3;

    // derivatives
    tile_data.ddx[0] = mat2mult(dCoordDx, cos_r_x, -sin_r_x, sin_r_x, cos_r_x) / scale1;
    tile_data.ddx[1] = mat2mult(dCoordDx, cos_r_y, -sin_r_y, sin_r_y, cos_r_y) / scale2;
    tile_data.ddx[2] = mat2mult(dCoordDx, cos_r_z, -sin_r_z, sin_r_z, cos_r_z) / scale3;
    tile_data.ddy[0] = mat2mult(dCoordDy, cos_r_x, -sin_r_x, sin_r_x, cos_r_x) / scale1;
    tile_data.ddy[1] = mat2mult(dCoordDy, cos_r_y, -sin_r_y, sin_r_y, cos_r_y) / scale2;
    tile_data.ddy[2] = mat2mult(dCoordDy, cos_r_z, -sin_r_z, sin_r_z, cos_r_z) / scale3;

    return tile_data;
}

}
}