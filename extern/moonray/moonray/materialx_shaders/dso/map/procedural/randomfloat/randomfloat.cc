// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file randomfloat.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>
#include <moonray/common/noise/Noise.h>
#include "constants.h"

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following variants.
// use #define VARIANT before including this file
// float, integer
//

#define OPERATION randomfloat
#define IN_DEFAULT_FLOAT 0.0f
#define IN_DEFAULT_INT 0

//
// Configure the variants
//
#if STRING_CMP(VARIANT,float)
#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT

#elif STRING_CMP(VARIANT,integer)
#define IN_TYPE Int
#define IN_DEFAULT IN_DEFAULT_INT

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

//
// Only need map shader
//
#include <moonray/rendering/shading/MapApi.h>
#define SHADER_TYPE Map
#define RETURN_TYPE Color


//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<IN_TYPE> inAttr;
    AttributeKey<Float>   minAttr;
    AttributeKey<Float>   maxAttr;
    AttributeKey<Int>     seedAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    inAttr       = sceneClass.declareAttribute<IN_TYPE>("in",   IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    minAttr      = sceneClass.declareAttribute<Float>  ("min",  0.0f,       FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    maxAttr      = sceneClass.declareAttribute<Float>  ("max",  1.0f,       FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    seedAttr     = sceneClass.declareAttribute<Int>    ("seed", 0,          FLAGS_BINDABLE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END


//
// end of attributes.cc
//

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const SHADER_TYPE *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, RETURN_TYPE *sample);
    ispc::SHADER_NAME mIspc;
    std::unique_ptr<moonray::noise::Noise> mNoise;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = TYPE_RGB;
}

void
SHADER_NAME::update()
{
    // This section copied from the cellnoise2d shader code.
    // Create Noise for cell noise - cell noise is just a hash-based noise
    // Use seed 0 and standard table size for consistent cell-based random values
    mNoise = std::make_unique<moonray::noise::Noise>(
        0,      // seed
        kTableSize,
        false   // useStaticTables
    );
    mIspc.mNoise = mNoise->getIspc();
}

void
SHADER_NAME::sample(const SHADER_TYPE* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

#if STRING_CMP(VARIANT,integer)
    const int in = evalInt(me, inAttr, tls, state);
#else
    const int in = static_cast<int>(floor(evalFloat(me, inAttr, tls, state) * kFloatScale));
#endif

    const float lo   = evalFloat(me, minAttr,  tls, state);
    const float hi   = evalFloat(me, maxAttr,  tls, state);
    const int   seed = evalInt  (me, seedAttr, tls, state);

    // This section adapted from the cellnoise2d shader code.
    // Cell noise returns a pseudo-random value for each cell based on (in, seed) pair.
    // Generate a unique cell ID using the noise permutation table.
    // Hash the cell coordinates through the permutation table.
    const ispc::NOISE_Noise* noise = me->mNoise->getIspc();
    const int tableSize = noise->mTableSize;
    const int sizeMask = tableSize - 1;
    const int* permTable = noise->mPermutationTable;
    const int cellId = permTable[(permTable[in & sizeMask] + seed) & sizeMask];
    
    // Convert the cell ID to a float in [0, 1] range
    const float t = static_cast<float>(cellId) / static_cast<float>(sizeMask);

    // Remap to [min, max] range
    const float out = lerp(lo, hi, t);

    //
    // for Map shaders the output type needs to be copied into a Color for
    // the return value.
    //
    *sample = Color(out);
}

