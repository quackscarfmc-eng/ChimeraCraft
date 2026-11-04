// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_tiledcloverleafs_color3.cc

#include "attributes.cc"
#include "ND_tiledcloverleafs_color3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_tiledcloverleafs_color3, Map)
public:
    ND_tiledcloverleafs_color3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *map, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_tiledcloverleafs_color3 mIspc;

RDL2_DSO_CLASS_END(ND_tiledcloverleafs_color3)

ND_tiledcloverleafs_color3::ND_tiledcloverleafs_color3(const SceneClass& sceneClass,
                                                       const std::string& name)
    : Parent(sceneClass, name)
{
    mSampleFunc = ND_tiledcloverleafs_color3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_tiledcloverleafs_color3_getSampleFunc();
}

void
ND_tiledcloverleafs_color3::update()
{
    mIspc.texcoordUseDefault = (!getBinding(texcoordAttr) && isDefault(texcoordAttr));
}

void
ND_tiledcloverleafs_color3::sample(const Map* map, moonray::shading::TLState *tls,
                                   const moonray::shading::State& state, Color* sample)
{
    const ND_tiledcloverleafs_color3* me = static_cast<const ND_tiledcloverleafs_color3*>(map);

    // Get attribute values
    const Vec2f texcoord  = me->mIspc.texcoordUseDefault ? state.getSt() : evalVec2f(me, texcoordAttr, tls, state);
    const Vec2f uvtiling  = evalVec2f(me, uvtilingAttr,  tls, state);
    const Vec2f uvoffset  = evalVec2f(me, uvoffsetAttr,  tls, state);
    const float size      = evalFloat(me, sizeAttr,      tls, state);
    const Bool  staggered = evalBool (me, staggeredAttr, tls, state);

    // Scale and offset so that 1 rectangular tile maps to [0,1]x[0,1] in uv space
    const Vec2f uv = texcoord * uvtiling - uvoffset;

    // Determine center of nearest cloverleaf
    Vec2f center;
    if (staggered) {
        // For this shader, "staggered" means offsetting alternate rows by half a tile width while simultaneously
        // respacing the rows so that the resulting pattern is a square lattice rotated 45 degrees. Thus we
        // perform the rounding-to-nearest in a suitably rotated coordinate system.

        // Rotate, scale and offset
        const Vec2f uv1 = Vec2f(uv.x + uv.y + 0.5f, uv.x - uv.y + 0.5f);

        // Round
        const Vec2f uv2 = Vec2f(floor(uv1.x), floor(uv1.y));

        // Undo rotation and scale to give center
        center = 0.5f * Vec2f(uv2.x + uv2.y, uv2.x - uv2.y);
    } else {
        // Not staggered - simply find center of square containing the uv-point.
        center = Vec2f(floor(uv.x) + 0.5f, floor(uv.y) + 0.5f);
    }

    // Do procedural calculation:
    // The cloverleaf shape is made of 4 overlapping disks - using abs() and max() picks the closest disk.
    const Vec2f duv = abs(uv - center);
    const float radius = 0.5f * size;
    const float out = (lengthSqr(duv) <= radius * max(duv.x, duv.y)) ? 1.0f : 0.0f;

    // Output result
    *sample = Color(out);
}

