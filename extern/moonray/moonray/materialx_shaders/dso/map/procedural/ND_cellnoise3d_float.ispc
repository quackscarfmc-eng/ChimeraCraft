// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_cellnoise3d_float.ispc

#include <moonray/rendering/shading/ispc/MapApi.isph>
#include <moonray/common/noise/ispc/Noise.isph>

struct ND_cellnoise3d_float {
    const NOISE_Noise *mNoise;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_cellnoise3d_float);

extern uniform Vec3fAttrKeyISPC* uniform positionAttr;

static varying Color
sample(const uniform Map*    uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform ND_cellnoise3d_float* uniform me = MAP_GET_ISPC_CPTR(ND_cellnoise3d_float, map);

    const varying Vec3f position = evalVec3f(map, tls, state, positionAttr);

    // Cell noise returns a pseudo-random value for each cell based on position
    // We use the integer cell coordinates to generate a consistent random value
    const varying int ix = (int)floor(position.x);
    const varying int iy = (int)floor(position.y);
    const varying int iz = (int)floor(position.z);
    
    // Generate a unique cell ID using the noise permutation table
    // Hash the cell coordinates through the permutation table
    const uniform int tableSize = me->mNoise->mTableSize;
    const uniform int* uniform permTable = me->mNoise->mPermutationTable;
    
    const varying int cellId = permTable[
        (permTable[
            (permTable[ix & (tableSize - 1)] + iy) & (tableSize - 1)
        ] + iz) & (tableSize - 1)
    ];
    
    // Convert the cell ID to a float in [0, 1) range
    varying float outValue = (float)cellId / (float)tableSize;

    return Color_ctor(outValue, outValue, outValue);
}

DEFINE_MAP_SHADER(ND_cellnoise3d_float, sample)
