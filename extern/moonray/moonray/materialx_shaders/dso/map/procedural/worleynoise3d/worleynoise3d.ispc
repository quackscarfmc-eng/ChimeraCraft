// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file worleynoise3d.ispc

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <moonray/common/noise/ispc/Worley.isph>

// supports the following variants (worleynoise3d only supports float, vector2, vector3)
// use #define VARIANT before including this file
// float, vector2, vector3
//

#define OPERATION worleynoise3d

//
// Configure the variants
// The output type is always the same as OUT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define OUT_TYPE Float
#define OUT_ATTRKEY FloatAttrKeyISPC

#elif STRING_CMP(VARIANT,vector2)
#define OUT_TYPE Vec2f
#define OUT_ATTRKEY Float2AttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)
#define OUT_TYPE Vec3f
#define OUT_ATTRKEY Float3AttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(OUT_TYPE,Float) || STRING_CMP(OUT_TYPE,Vec2f) || STRING_CMP(OUT_TYPE,Vec3f)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE OUT_TYPE
#endif

struct SHADER_NAME {
    const NOISE_Worley *mWorley;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform FloatAttrKeyISPC* uniform jitterAttr;
extern uniform Float3AttrKeyISPC* uniform positionAttr;

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    const varying Float jitter = evalFloat(map, tls, state, jitterAttr);
    const varying Vec3f position = evalVec3f(map, tls, state, positionAttr);

    // Create Worley noise sample structure
    varying NOISE_WorleySample noiseSample;
    noiseSample.position = position;
    
    // Create array for worley points
    varying NOISE_WorleyPoint worleyPoints[NOISE_WORLEY_MAX_SEARCH_POINTS];
    
    // Search for points with jitter using fractal method
    const varying float minkowskiNumber = 2.0f; // Standard Euclidean distance
    const varying float maxLevel = 1.0f; // Single level for basic Worley noise
    NOISE_worleySearchPointsFractal(
        me->mWorley,
        tls,
        jitter,
        minkowskiNumber,
        maxLevel,
        noiseSample,
        worleyPoints
    );
    
    // The points are already sorted by distance (first 4)
    // Check if we have valid points
    varying int numPoints = 0;
    for (int i = 0; i < 4; ++i) {
        if (worleyPoints[i].dist >= 0.0f) {
            numPoints = i + 1;
        } else {
            break;
        }
    }
    
    varying OUT_TYPE outValue;

#if STRING_CMP(VARIANT,float)
    // For float output, return the distance to the closest point
    // Apply square root for better distribution, then clamp to [0,1] range
    // The sqrt reduces contrast and matches Karma's Worley noise implementation
    if (numPoints > 0) {
        outValue = clamp(sqrt(worleyPoints[0].dist), 0.0f, 1.0f);
    } else {
        outValue = 0.0f;
    }
#elif STRING_CMP(VARIANT,vector2)
    // For vector2 output, return distances to the two closest points
    // Apply square root for better distribution, then clamp each component to [0,1] range
    // The sqrt reduces contrast and matches Karma's Worley noise implementation
    if (numPoints > 0) {
        outValue.x = clamp(sqrt(worleyPoints[0].dist), 0.0f, 1.0f);
        outValue.y = (numPoints > 1) ? clamp(sqrt(worleyPoints[1].dist), 0.0f, 1.0f) : 0.0f;
    } else {
        outValue.x = 0.0f;
        outValue.y = 0.0f;
    }
#elif STRING_CMP(VARIANT,vector3)
    // For vector3 output, return distances to the three closest points
    // Apply square root for better distribution, then clamp each component to [0,1] range
    // The sqrt reduces contrast and matches Karma's Worley noise implementation
    if (numPoints > 0) {
        outValue.x = clamp(sqrt(worleyPoints[0].dist), 0.0f, 1.0f);
        outValue.y = (numPoints > 1) ? clamp(sqrt(worleyPoints[1].dist), 0.0f, 1.0f) : 0.0f;
        outValue.z = (numPoints > 2) ? clamp(sqrt(worleyPoints[2].dist), 0.0f, 1.0f) : 0.0f;
    } else {
        outValue.x = 0.0f;
        outValue.y = 0.0f;
        outValue.z = 0.0f;
    }
#endif
        
    //
    // convert to the output type if necessary
    //
#if STRING_CMP(OUT_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(OUT_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(OUT_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}

// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
