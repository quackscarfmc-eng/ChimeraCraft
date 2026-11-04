// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "randomfloat.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <moonray/common/noise/ispc/Noise.isph>
#include "attributes.isph"
#include "constants.h"

// supports the following variants.
// use #define VARIANT before including this file
// float, integer
//

#define OPERATION randomfloat

//
// Configure the variants
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,integer)
#define IN_TYPE Int
#define IN_ATTRKEY IntAttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

//
// Only need map shader
//
#include <moonray/rendering/shading/ispc/MapApi.isph>
#define SHADER_TYPE Map
#define RETURN_TYPE Color


struct SHADER_NAME {
    const NOISE_Noise *mNoise;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;
extern uniform FloatAttrKeyISPC* uniform minAttr;
extern uniform FloatAttrKeyISPC* uniform maxAttr;
extern uniform IntAttrKeyISPC* uniform seedAttr;

static varying RETURN_TYPE
sample(const uniform SHADER_TYPE*    uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

#if STRING_CMP(VARIANT,integer)
    const varying int inValue = evalInt(map, tls, state, inAttr);
#else
    const varying int inValue = (int)floor(evalFloat(map, tls, state, inAttr) * kFloatScale);
#endif

    const varying float lo   = evalFloat(map, tls, state, minAttr);
    const varying float hi   = evalFloat(map, tls, state, maxAttr);
    const varying int   seed = evalInt  (map, tls, state, seedAttr);

    // This section adapted from the cellnoise2d shader code.
    // Cell noise returns a pseudo-random value for each cell based on (inValue, seed) pair.
    // Generate a unique cell ID using the noise permutation table.
    // Hash the cell coordinates through the permutation table.
    const uniform int tableSize = me->mNoise->mTableSize;
    const uniform int sizeMask = tableSize - 1;
    const uniform int* permTable = me->mNoise->mPermutationTable;
    const varying int cellId = permTable[(permTable[inValue & sizeMask] + seed) & sizeMask];
    
    // Convert the cell ID to a float in [0, 1] range
    const varying float t = (float)cellId / (float)sizeMask;

    // Remap to [min, max] range
    const varying float out = lerp(lo, hi, t);

    //
    // for Map shaders the output type needs to be copied into a Color for
    // the return value.
    //
    return Color_ctor(out);
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname) DEFINE_MAP_SHADER(shadername, funcname)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample)

