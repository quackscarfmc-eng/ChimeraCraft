// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file worleynoise2d.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <moonray/common/noise/Worley.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants (worleynoise2d only supports float, vector2, vector3)
// use #define VARIANT before including this file
// float, vector2, vector3
//

#define OPERATION worleynoise2d
#define JITTER_DEFAULT_FLOAT 1.0f
#define TEXCOORD_DEFAULT_VEC2F Vec2f(0.0f, 0.0f)

//
// Configure the variants
// The output type is always the same as OUT_TYPE
//
#if STRING_CMP(VARIANT,float)
#define OUT_TYPE Float
#define OUT_DEFAULT 0.0f
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2)
#define OUT_TYPE Vec2f
#define OUT_DEFAULT Vec2f(0.0f, 0.0f)
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)
#define OUT_TYPE Vec3f
#define OUT_DEFAULT Vec3f(0.0f, 0.0f, 0.0f)
#define OUT_ATTRIBUTE_TYPE TYPE_RGB

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define OUT_EVAL CONCAT2(eval,OUT_TYPE)

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(OUT_TYPE,Float) || STRING_CMP(OUT_TYPE,Vec2f) || STRING_CMP(OUT_TYPE,Vec3f)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE OUT_TYPE
#endif

//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Float> jitterAttr;
    AttributeKey<Vec2f> texcoordAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    jitterAttr = sceneClass.declareAttribute<Float>("jitter", JITTER_DEFAULT_FLOAT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    texcoordAttr = sceneClass.declareAttribute<Vec2f>("texcoord", TEXCOORD_DEFAULT_VEC2F, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END

//
// end of attributes.cc
//

//
// Shader class declaration
//
RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);

    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, RETURN_TYPE *sample);

    ispc::SHADER_NAME mIspc;
    std::unique_ptr<moonray::noise::Worley> mWorley;

RDL2_DSO_CLASS_END(SHADER_NAME)

//
// Shader class implementation
//
SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
                         const std::string& name) :
    Parent(sceneClass, name)
{
    // the functions pointers are unions so it doesn't
    // really matter which one is assigned to and the variability
    // is easier to manage with macros with picking one and forcing
    // with a cast
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
	mOutputType = OUT_ATTRIBUTE_TYPE;
}

void
SHADER_NAME::update()
{
    // Initialize Worley noise with default parameters
    mWorley = std::make_unique<moonray::noise::Worley>();
    mIspc.mWorley = mWorley->getIspcWorley();
}

void
SHADER_NAME::sample(const Map* self,
                    moonray::shading::TLState *tls,
                    const moonray::shading::State& state,
                    RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const Float jitter = evalFloat(me, jitterAttr, tls, state);
    const Vec2f texcoord = evalVec2f(me, texcoordAttr, tls, state);

    // Convert 2D texcoord to 3D position for Worley calculation
    const Vec3f position = Vec3f(texcoord.x, texcoord.y, 0.0f);

    // Create Worley noise sample structure
    ispc::NOISE_WorleySample noiseSample;
    noiseSample.position = asIspc(position);
    
    // Create array for worley points
    moonray::noise::Worley_PointArray worleyPoints;
    
    // Search for points with jitter using fractal method
    const float minkowskiNumber = 2.0f; // Standard Euclidean distance
    const float maxLevel = 1.0f; // Single level for basic Worley noise
    me->mWorley->searchPointsFractal(
        jitter,
        minkowskiNumber,
        maxLevel,
        noiseSample,
        worleyPoints
    );
    
    // The points are already sorted by distance
    // Check if we have valid points (first 4 are sorted)
    int numPoints = 0;
    for (int i = 0; i < 4 && i < static_cast<int>(worleyPoints.size()); ++i) {
        if (worleyPoints[i].dist >= 0.0f) {
            numPoints = i + 1;
        } else {
            break;
        }
    }
    
    OUT_TYPE outValue;

#if STRING_CMP(VARIANT,float)
    // For float output, return the distance to the closest point
    // Apply square root for better distribution, then clamp to [0,1] range
    // The sqrt reduces contrast and matches Karma's Worley noise implementation
    if (numPoints > 0) {
        outValue = scene_rdl2::math::clamp(scene_rdl2::math::sqrt(worleyPoints[0].dist), 0.0f, 1.0f);
    } else {
        outValue = 0.0f;
    }
#elif STRING_CMP(VARIANT,vector2)
    // For vector2 output, return distances to the two closest points
    // Apply square root for better distribution, then clamp each component to [0,1] range
    // The sqrt reduces contrast and matches Karma's Worley noise implementation
    if (numPoints > 0) {
        outValue.x = scene_rdl2::math::clamp(scene_rdl2::math::sqrt(worleyPoints[0].dist), 0.0f, 1.0f);
        outValue.y = (numPoints > 1) ? scene_rdl2::math::clamp(scene_rdl2::math::sqrt(worleyPoints[1].dist), 0.0f, 1.0f) : 0.0f;
    } else {
        outValue.x = 0.0f;
        outValue.y = 0.0f;
    }
#elif STRING_CMP(VARIANT,vector3)
    // For vector3 output, return distances to the three closest points
    // Apply square root for better distribution, then clamp each component to [0,1] range
    // The sqrt reduces contrast and matches Karma's Worley noise implementation
    if (numPoints > 0) {
        outValue.x = scene_rdl2::math::clamp(scene_rdl2::math::sqrt(worleyPoints[0].dist), 0.0f, 1.0f);
        outValue.y = (numPoints > 1) ? scene_rdl2::math::clamp(scene_rdl2::math::sqrt(worleyPoints[1].dist), 0.0f, 1.0f) : 0.0f;
        outValue.z = (numPoints > 2) ? scene_rdl2::math::clamp(scene_rdl2::math::sqrt(worleyPoints[2].dist), 0.0f, 1.0f) : 0.0f;
    } else {
        outValue.x = 0.0f;
        outValue.y = 0.0f;
        outValue.z = 0.0f;
    }
#endif

    //
    // Convert to the output type if necessary
#if STRING_CMP(OUT_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(OUT_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(OUT_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}
