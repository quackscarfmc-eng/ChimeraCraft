// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_tiledhexagons_color3.cc

#include "attributes.cc"
#include "ND_tiledhexagons_color3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_tiledhexagons_color3, Map)
public:
    ND_tiledhexagons_color3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *map, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_tiledhexagons_color3 mIspc;

RDL2_DSO_CLASS_END(ND_tiledhexagons_color3)

ND_tiledhexagons_color3::ND_tiledhexagons_color3(const SceneClass& sceneClass,
                                                 const std::string& name)
    : Parent(sceneClass, name)
{
    mSampleFunc = ND_tiledhexagons_color3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_tiledhexagons_color3_getSampleFunc();
}

void
ND_tiledhexagons_color3::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_tiledhexagons_color3::sample(const Map* map, moonray::shading::TLState *tls,
                                const moonray::shading::State& state, Color* sample)
{
    const ND_tiledhexagons_color3* me = static_cast<const ND_tiledhexagons_color3*>(map);

    // Get attribute values
    const Vec2f texcoord  = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);
    const Vec2f uvtiling  = evalVec2f(me, uvtilingAttr, tls, state);
    const Vec2f uvoffset  = evalVec2f(me, uvoffsetAttr, tls, state);
    const float size      = evalFloat(me, sizeAttr, tls, state);
    const Bool  staggered = evalBool (me, staggeredAttr, tls, state);

    // Scale and offset so that 1 rectangular tile maps to [0,1]x[0,1] in uv space
    const Vec2f uv = texcoord * uvtiling - uvoffset;

    float maxDistance;
    if (staggered) {
        // Use hexagonal lattice.
        // Indices i=0,1,...,11 correspond to direction vectors rotated 30*i degrees counterclockwise from the x-axis.
        // The following logic determines which hexagonal cell the point (uv) falls into.
        // The hexagonal grid can be described using three sets of parallel lines, each set at 120 degrees to the
        // others. We project the point onto three axes (directions 0, 4, and 8), which are separated by 120 degrees.
        // The projections (d0, d4, d8) are scaled so that the distance between adjacent lines is 1.
        // Taking the floor of these projections gives us the indices of the nearest lines in each direction.
        // The combinations of these indices define the hexagonal cells.

        // Take (scaled) dot products with directions 0, 4, 8:
        float d0 =  2.0f * uv.x;                    // dot uv with 2 * (1, 0)
        float d4 = -uv.x + 1.73205080757f * uv.y;   // dot uv with 2 * (-1/2,  sqrt(3)/2)
        float d8 = -uv.x - 1.73205080757f * uv.y;   // dot uv with 2 * (-1/2, -sqrt(3)/2)

        // Find floors corresponding to cell boundaries
        const float i0 = floor(d0);
        const float i4 = floor(d4);
        const float i8 = floor(d8);

        // Reduce dot products to their fractional values
        d0 -= i0;
        d4 -= i4;
        d8 -= i8;

        // Pick out the 120-degree pie slice containing the uv point and take the max of the 2 corresponding distances
        if ((int)(i0 - i8 - 1.0f) % 3 == 0) {
            maxDistance = max(d0, 1.0f - d8);
        } else if ((int)(i4 - i0 - 1.0f) % 3 == 0) {
            maxDistance = max(d4, 1.0f - d0);
        } else {
            maxDistance = max(d8, 1.0f - d4);
        }

    } else {
        // Not staggered; use square lattice.
        // Simply find center of square containing the uv-point.
        const Vec2f center = Vec2f(floor(uv.x) + 0.5f, floor(uv.y) + 0.5f);

        // Find position relative to center, and take (scaled) dot products with directions 0, 4, 8
        const Vec2f duv = uv - center;
        const float d0 =  2.0f * duv.x;
        const float d4 = -duv.x + 1.73205080757f * duv.y;
        const float d8 = -duv.x - 1.73205080757f * duv.y;

        // Take the max of the distances in all 6 directions
        const float m0 = max(d0, -d0);
        const float m4 = max(d4, -d4);
        const float m8 = max(d8, -d8);
        maxDistance = max(m0, max(m4, m8));
    }

    const float out = (maxDistance < size) ? 1.0f : 0.0f;

    // Output result
    *sample = Color(out);
}

