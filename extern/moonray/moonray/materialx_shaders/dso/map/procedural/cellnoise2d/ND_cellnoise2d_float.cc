// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_cellnoise2d_float.cc

#include "attributes.cc"
#include "ND_cellnoise2d_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
#include <moonray/common/noise/Noise.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_cellnoise2d_float, Map)
public:
    ND_cellnoise2d_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_cellnoise2d_float mIspc;
    std::unique_ptr<moonray::noise::Noise> mNoise;

RDL2_DSO_CLASS_END(ND_cellnoise2d_float)

ND_cellnoise2d_float::ND_cellnoise2d_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_cellnoise2d_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_cellnoise2d_float_getSampleFunc();
}

void
ND_cellnoise2d_float::update()
{
    // Create Noise for cell noise - cell noise is just a hash-based noise
    // Use seed 0 and standard table size for consistent cell-based random values
    mNoise = std::make_unique<moonray::noise::Noise>(
        0,      // seed
        2048,   // tableSize
        false   // useStaticTables
    );
    mIspc.mNoise = mNoise->getIspc();
}

void
ND_cellnoise2d_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_cellnoise2d_float* me = static_cast<const ND_cellnoise2d_float*>(self);

    const Vec2f texcoord = evalVec2f(me, texcoordAttr, tls, state);

    // Cell noise returns a pseudo-random value for each cell based on texcoord
    // We use the integer cell coordinates to generate a consistent random value
    const int ix = static_cast<int>(floor(texcoord.x));
    const int iy = static_cast<int>(floor(texcoord.y));
    
    // Generate a unique cell ID using the noise permutation table
    // Hash the cell coordinates through the permutation table
    const ispc::NOISE_Noise* noise = me->mNoise->getIspc();
    const int tableSize = noise->mTableSize;
    const int* permTable = noise->mPermutationTable;
    
    const int cellId = permTable[
        (permTable[ix & (tableSize - 1)] + iy) & (tableSize - 1)
    ];
    
    // Convert the cell ID to a float in [0, 1) range
    float outValue = static_cast<float>(cellId) / static_cast<float>(tableSize);

    *sample = Color(outValue, outValue, outValue);
}
