// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "MtlxHextile.ispc"

#include "MtlxHextile.isph"


// Christophe Schlick. “Fast Alternatives to Perlin’s Bias and Gain Functions”.
// In Graphics Gems IV, Morgan Kaufmann, 1994, pages 401–403.
// https://dept-info.labri.fr/~schlick/DOC/gem2.html
varying float
mxSchlickGain(varying float x, varying float r)
{
    const varying float rr = clamp(r, 0.001f, 0.999f);
    const varying float a = (1.0f / rr - 2.0f) * (1.0f - 2.0f * x);
    return (x < 0.5f) ? x / (a + 1.0f) : (a - x) / (a - 1.0f);
}

// Cheap hash without Sine, Dave Hoskins
// https://www.shadertoy.com/view/4djSRW
varying Vec2f
mxHextileHash(const varying Vec2f& p)
{
    varying Vec3f p3 = Vec3f_ctor(0.1031f * p.x,
                                  0.1030f * p.y,
                                  0.0973f * p.x);
    p3.x -= floor(p3.x);
    p3.y -= floor(p3.y);
    p3.z -= floor(p3.z);
    const varying float f = dot(p3,
                                Vec3f_ctor(33.33f + p3.y,
                                           33.33f + p3.z,
                                           33.33f + p3.x));
    p3 = p3 + Vec3f_ctor(f, f, f);
    varying Vec2f out;
    out.x = p3.x + p3.y;
    out.y = p3.x + p3.z;
    out.x *= p3.z;
    out.y *= p3.y;
    out.x -= floor(out.x);
    out.y -= floor(out.y);
    return out;
}


varying Vec3f
HextileComputeBlendWeights(const varying Vec3f& luminanceWeights,
                           const varying Vec3f& tileWeights,
                           const varying float falloff)
{
    const varying Vec3f t3 = tileWeights * tileWeights * tileWeights;
    varying Vec3f w = luminanceWeights * tileWeights * t3 * t3;
    varying float norm = 1.0f / (w.x + w.y + w.z);
    w = norm * w;

    if (falloff != 0.5f)
    {
        w.x = mxSchlickGain(w.x, falloff);
        w.y = mxSchlickGain(w.y, falloff);
        w.z = mxSchlickGain(w.z, falloff);
        norm = 1.0f / (w.x + w.y + w.z);
        w = norm * w;
    }

    return w;
}


varying Vec2f
hexCenter(const varying Vec2f& v)
{
    // skew using this matrix:
    // 1.0   0.5
    // 0.0   root(3)/2
    
    const float r3_2 = 0.86602f;
    const float two_r3 = 3.4641f;
    varying Vec2f vf = v / two_r3;
    Vec2f out;
    out.x = vf.x + 0.5f * vf.y;
    out.y = r3_2 * vf.y;

    return out;
}

void
HextileCoord(const varying Vec2f& coord,
             const varying float rotation,
             const varying Vec2f& rotationRange,
             const varying float scale,
             const varying Vec2f& scaleRange,
             const varying float offset,
             const varying Vec2f& offsetRange,
             const varying const Vec2f& dCoordDx,
             const varying Vec2f& dCoordDy,
                   varying HextileData& me)
{
    const uniform float sqrt3_2 = sqrt(3.0f) * 2.0f;

    // scale coord to maintain the original fit
    const varying Vec2f st = coord * sqrt3_2;

    // skew input space into simplex triangle grid
    // (1, 0, -tan(30), 2*tan(30))
    const varying Vec2f stSkewed = mat2mult(st, 1.0f, 0.0f, -0.57735027f, 1.15470054f);

    // barycentric weights
    const varying Vec2f stFrac = Vec2f_ctor(stSkewed.x - floor(stSkewed.x),
                                            stSkewed.y - floor(stSkewed.y));

    const varying Vec3f temp = Vec3f_ctor(stFrac.x,
                                          stFrac.y,
                                          1.0 - stFrac.x - stFrac.y);

    const varying float s = -temp.z < 0.0f ? 0.0f : 1.0f;
    const varying float s2 = 2.0f * s - 1.0f;

    const varying float w1 = -temp.z * s2;
    const varying float w2 = s - temp.y * s2;
    const varying float w3 = s - temp.x * s2;

    // vertex IDs
    const varying Vec2f baseID = Vec2f_ctor(floor(stSkewed.x), floor(stSkewed.y));
    
    const varying Vec2f id1 = baseID + Vec2f_ctor(s, s);
    const varying Vec2f id2 = baseID + Vec2f_ctor(s, 1 - s);
    const varying Vec2f id3 = baseID + Vec2f_ctor(1 - s, s);

    // tile center
    const varying Vec2f ctr1 = mat2mult(id1 / Vec2f_ctor(sqrt3_2), 1.0f, 0.0f, 0.5f, 1.0f / 1.15470054f);
    const varying Vec2f ctr2 = mat2mult(id2 / Vec2f_ctor(sqrt3_2), 1.0f, 0.0f, 0.5f, 1.0f / 1.15470054f);
    const varying Vec2f ctr3 = mat2mult(id3 / Vec2f_ctor(sqrt3_2), 1.0f, 0.0f, 0.5f, 1.0f / 1.15470054f);

    // reuse hash for performance
    const uniform Vec2f seed_offset = Vec2f_ctor(0.12345f);  // to avoid some zeros
    const varying Vec2f rand1 = mxHextileHash(id1 + seed_offset);
    const varying Vec2f rand2 = mxHextileHash(id2 + seed_offset);
    const varying Vec2f rand3 = mxHextileHash(id3 + seed_offset);

    // randomized rotation matrix
    const varying float rr_x = rotationRange.x * sPi / 180.0f;
    const varying float rr_y = rotationRange.y * sPi / 180.0f;
    const varying Vec3f rand_x = Vec3f_ctor(rand1.x, rand2.x, rand3.x);
    const varying Vec3f rotations = lerp(Vec3f_ctor(rr_x), Vec3f_ctor(rr_y), rand_x * rotation);
    varying float sin_r_x;
    varying float sin_r_y;
    varying float sin_r_z;
    varying float cos_r_x;
    varying float cos_r_y;
    varying float cos_r_z;

    sincos(rotations.x, &sin_r_x, &cos_r_x);
    sincos(rotations.y, &sin_r_y, &cos_r_y);
    sincos(rotations.z, &sin_r_z, &cos_r_z);

    // randomized scale
    const varying Vec3f rand_y = Vec3f_ctor(rand1.y, rand2.y, rand3.y);
    const varying Vec3f scales = lerp(Vec3f_ctor(1.0f),
                                      lerp(Vec3f_ctor(scaleRange.x),
                                           Vec3f_ctor(scaleRange.y),
                                           rand_y),
                                      scale);
    const varying Vec2f scale1 = Vec2f_ctor(scales.x);
    const varying Vec2f scale2 = Vec2f_ctor(scales.y);
    const varying Vec2f scale3 = Vec2f_ctor(scales.z);

    // randomized offset
    const varying Vec2f offset1 = lerp(Vec2f_ctor(offsetRange.x), Vec2f_ctor(offsetRange.y), rand1 * offset);
    const varying Vec2f offset2 = lerp(Vec2f_ctor(offsetRange.x), Vec2f_ctor(offsetRange.y), rand2 * offset);
    const varying Vec2f offset3 = lerp(Vec2f_ctor(offsetRange.x), Vec2f_ctor(offsetRange.y), rand3 * offset);

    me.weights = Vec3f_ctor(w1, w2, w3);
    me.rotations = rotations;

    // get coord
    me.coords[0] = mat2mult(coord - ctr1, cos_r_x, -sin_r_x, sin_r_x, cos_r_x) / scale1 + ctr1 + offset1;
    me.coords[1] = mat2mult(coord - ctr2, cos_r_y, -sin_r_y, sin_r_y, cos_r_y) / scale2 + ctr2 + offset2;
    me.coords[2] = mat2mult(coord - ctr3, cos_r_z, -sin_r_z, sin_r_z, cos_r_z) / scale3 + ctr3 + offset3;

    // derivatives
    me.ddx[0] = mat2mult(dCoordDx, cos_r_x, -sin_r_x, sin_r_x, cos_r_x) / scale1;
    me.ddx[1] = mat2mult(dCoordDx, cos_r_y, -sin_r_y, sin_r_y, cos_r_y) / scale2;
    me.ddx[2] = mat2mult(dCoordDx, cos_r_z, -sin_r_z, sin_r_z, cos_r_z) / scale3;
    me.ddy[0] = mat2mult(dCoordDy, cos_r_x, -sin_r_x, sin_r_x, cos_r_x) / scale1;
    me.ddy[1] = mat2mult(dCoordDy, cos_r_y, -sin_r_y, sin_r_y, cos_r_y) / scale2;
    me.ddy[2] = mat2mult(dCoordDy, cos_r_z, -sin_r_z, sin_r_z, cos_r_z) / scale3;
}

