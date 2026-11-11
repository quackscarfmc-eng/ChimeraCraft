// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_unifiednoise2d_float.ispc

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>
#include <moonray/common/noise/ispc/Perlin.isph>
#include <moonray/common/noise/ispc/Noise.isph>
#include <moonray/common/noise/ispc/Worley.isph>

struct ND_unifiednoise2d_float {
    const NOISE_Perlin *mPerlinNoise;
    const NOISE_Noise *mCellNoise;
    const NOISE_Worley *mWorleyNoise;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_unifiednoise2d_float);

static varying Color
sample(const uniform Map*    uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform ND_unifiednoise2d_float* uniform me = MAP_GET_ISPC_CPTR(ND_unifiednoise2d_float, map);

    const varying Vec2f texcoord = evalVec2f(map, tls, state, texcoordAttr);
    const varying Vec2f freq = evalVec2f(map, tls, state, freqAttr);
    const varying Vec2f offset = evalVec2f(map, tls, state, offsetAttr);
    const varying float jitter = evalFloat(map, tls, state, jitterAttr);
    const varying float outmin = evalFloat(map, tls, state, outminAttr);
    const varying float outmax = evalFloat(map, tls, state, outmaxAttr);
    const varying bool clampoutput = evalBool(map, tls, state, clampoutputAttr);
    const varying int octaves = evalInt(map, tls, state, octavesAttr);
    const varying float lacunarity = evalFloat(map, tls, state, lacunarityAttr);
    const varying float diminish = evalFloat(map, tls, state, diminishAttr);
    const uniform int type = getTypeAttr(map);

    // Apply frequency and offset transformations
    varying Vec2f transformedTexcoord = (texcoord * freq) + offset;
    varying Vec3f transformedPos = Vec3f_ctor(transformedTexcoord.x, transformedTexcoord.y, 0.0f);

    varying float outValue = 0.0f;

    // Switch based on noise type
    if (type == 0) { // Perlin noise
        // Use single octave perlin noise
        const int maxLevel = 1;
        const float persistence = 0.5f;
        const float lac = 2.0f;
        outValue = NOISE_perlinFractal3D(me->mPerlinNoise, transformedPos, maxLevel, persistence, lac);
        
        // Remap from [-1, 1] to [0, 1]
        outValue = (outValue + 1.0f) * 0.5f;
    } 
    else if (type == 1) { // Cell noise
        // Cell noise returns a pseudo-random value for each cell based on texcoord
        const varying int ix = (int)floor(transformedPos.x);
        const varying int iy = (int)floor(transformedPos.y);
        const varying int iz = (int)floor(transformedPos.z);
        
        // Generate a unique cell ID using the noise permutation table
        const uniform int tableSize = me->mCellNoise->mTableSize;
        const uniform int* uniform permTable = me->mCellNoise->mPermutationTable;
        
        const varying int cellId = permTable[
            (permTable[
                (permTable[ix & (tableSize - 1)] + iy) & (tableSize - 1)
            ] + iz) & (tableSize - 1)
        ];
        
        // Convert the cell ID to a float in [0, 1) range
        outValue = (float)cellId / (float)tableSize;
    } 
    else if (type == 2) { // Worley noise
        // Create Worley noise sample structure
        varying NOISE_WorleySample noiseSample;
        noiseSample.position = transformedPos;
        
        // Create array for worley points
        varying NOISE_WorleyPoint worleyPoints[NOISE_WORLEY_MAX_SEARCH_POINTS];
        
        // Search for points with jitter using fractal method
        const varying float minkowskiNumber = 2.0f; // Standard Euclidean distance
        const varying float maxLevel = 1.0f; // Single level for basic Worley noise
        NOISE_worleySearchPointsFractal(
            me->mWorleyNoise,
            tls,
            jitter,
            minkowskiNumber,
            maxLevel,
            noiseSample,
            worleyPoints
        );
        
        // For float output, return the distance to the closest point
        // Apply square root for better distribution
        if (worleyPoints[0].dist >= 0.0f) {
            outValue = sqrt(worleyPoints[0].dist);
        } else {
            outValue = 0.0f;
        }
    } 
    else if (type == 3) { // Fractal noise
        outValue = NOISE_perlinFractal3D(me->mPerlinNoise, transformedPos, octaves, diminish, lacunarity);
        
        // Note: fractal noise output is not remapped from [-1, 1] to [0, 1] to match Karma's behavior
    }

    // Remap from [0, 1] to [outmin, outmax]
    outValue = outmin + outValue * (outmax - outmin);
    
    // Optionally clamp the output
    if (clampoutput) {
        outValue = clamp(outValue, outmin, outmax);
    }

    return Color_ctor(outValue, outValue, outValue);
}

DEFINE_MAP_SHADER(ND_unifiednoise2d_float, sample)
