// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_unifiednoise2d_float.cc

#include "attributes.cc"
#include "ND_unifiednoise2d_float_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
#include <moonray/common/noise/Perlin.h>
#include <moonray/common/noise/Noise.h>
#include <moonray/common/noise/Worley.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_unifiednoise2d_float, Map)
public:
    ND_unifiednoise2d_float(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_unifiednoise2d_float mIspc;
    std::unique_ptr<moonray::noise::Perlin> mPerlinNoise;
    std::unique_ptr<moonray::noise::Noise> mCellNoise;
    std::unique_ptr<moonray::noise::Worley> mWorleyNoise;

RDL2_DSO_CLASS_END(ND_unifiednoise2d_float)

ND_unifiednoise2d_float::ND_unifiednoise2d_float(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_unifiednoise2d_float::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_unifiednoise2d_float_getSampleFunc();
}

void
ND_unifiednoise2d_float::update()
{
    // Initialize all noise types
    mPerlinNoise = std::make_unique<moonray::noise::Perlin>();
    mIspc.mPerlinNoise = mPerlinNoise->getIspcPerlin();
    
    // Create Noise for cell noise
    mCellNoise = std::make_unique<moonray::noise::Noise>(
        0,      // seed
        2048,   // tableSize
        false   // useStaticTables
    );
    mIspc.mCellNoise = mCellNoise->getIspc();
    
    // Initialize Worley noise
    mWorleyNoise = std::make_unique<moonray::noise::Worley>();
    mIspc.mWorleyNoise = mWorleyNoise->getIspcWorley();
}

void
ND_unifiednoise2d_float::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_unifiednoise2d_float* me = static_cast<const ND_unifiednoise2d_float*>(self);

    const Vec2f texcoord = evalVec2f(me, texcoordAttr, tls, state);
    const Vec2f freq = evalVec2f(me, freqAttr, tls, state);
    const Vec2f offset = evalVec2f(me, offsetAttr, tls, state);
    const Float jitter = evalFloat(me, jitterAttr, tls, state);
    const Float outmin = evalFloat(me, outminAttr, tls, state);
    const Float outmax = evalFloat(me, outmaxAttr, tls, state);
    const Bool clampoutput = evalBool(me, clampoutputAttr, tls, state);
    const Int octaves = evalInt(me, octavesAttr, tls, state);
    const Float lacunarity = evalFloat(me, lacunarityAttr, tls, state);
    const Float diminish = evalFloat(me, diminishAttr, tls, state);
    const Int type = evalInt(me, typeAttr, tls, state);

    // Apply frequency and offset transformations
    Vec2f transformedTexcoord = (texcoord * freq) + offset;
    Vec3f transformedPos(transformedTexcoord.x, transformedTexcoord.y, 0.0f);

    Float outValue = 0.0f;

    // Switch based on noise type
    switch (type) {
        case 0: { // Perlin noise
            // Use single octave perlin noise
            const int maxLevel = 1;
            const float persistence = 0.5f;
            const float lac = 2.0f;
            outValue = me->mPerlinNoise->perlinFractal3D(transformedPos, maxLevel, persistence, lac);
            
            // Remap from [-1, 1] to [0, 1]
            outValue = (outValue + 1.0f) * 0.5f;
            break;
        }
        
        case 1: { // Cell noise
            // Cell noise returns a pseudo-random value for each cell based on texcoord
            const int ix = static_cast<int>(floor(transformedPos.x));
            const int iy = static_cast<int>(floor(transformedPos.y));
            const int iz = static_cast<int>(floor(transformedPos.z));
            
            // Generate a unique cell ID using the noise permutation table
            const ispc::NOISE_Noise* noise = me->mCellNoise->getIspc();
            const int tableSize = noise->mTableSize;
            const int* permTable = noise->mPermutationTable;
            
            const int cellId = permTable[
                (permTable[
                    (permTable[ix & (tableSize - 1)] + iy) & (tableSize - 1)
                ] + iz) & (tableSize - 1)
            ];
            
            // Convert the cell ID to a float in [0, 1) range
            outValue = static_cast<float>(cellId) / static_cast<float>(tableSize);
            break;
        }
        
        case 2: { // Worley noise
            // Create Worley noise sample structure
            ispc::NOISE_WorleySample noiseSample;
            noiseSample.position = asIspc(transformedPos);
            
            // Create array for worley points
            moonray::noise::Worley_PointArray worleyPoints;
            
            // Search for points with jitter using fractal method
            const float minkowskiNumber = 2.0f; // Standard Euclidean distance
            const float maxLevel = 1.0f; // Single level for basic Worley noise
            me->mWorleyNoise->searchPointsFractal(
                jitter,
                minkowskiNumber,
                maxLevel,
                noiseSample,
                worleyPoints
            );
            
            // For float output, return the distance to the closest point
            // Apply square root for better distribution, then clamp to [0,1] range
            if (worleyPoints.size() > 0 && worleyPoints[0].dist >= 0.0f) {
                outValue = scene_rdl2::math::sqrt(worleyPoints[0].dist);
            } else {
                outValue = 0.0f;
            }
            break;
        }
        
        case 3: { // Fractal noise
            outValue = me->mPerlinNoise->perlinFractal3D(transformedPos, octaves, diminish, lacunarity);
            
            // Note: fractal noise output is not remapped from [-1, 1] to [0, 1] to match Karma's behavior
            break;
        }
        
        default:
            outValue = 0.0f;
            break;
    }

    // Remap from [0, 1] to [outmin, outmax]
    outValue = outmin + outValue * (outmax - outmin);
    
    // Optionally clamp the output
    if (clampoutput) {
        outValue = scene_rdl2::math::clamp(outValue, outmin, outmax);
    }

    *sample = Color(outValue, outValue, outValue);
}
