// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "transformmatrix.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// Supports the following variants/operations
//      - transformnormal_vector3
//      - transformpoint_vector3
//      - transformvector_vector3
// Use #define VARIANT and OPERATION before including this file

// OPERATIONS
// (starting at 200 to avoid conflicting with MtlxVariantTypes)
#define UNIQUE_ID_transformnormal       200
#define UNIQUE_ID_transformvector       201
#define UNIQUE_ID_transformpoint        202

//
// Configure the defaults
//
#if STRING_CMP(OPERATION,transformnormal)
#define TRANSFORM_FUNC transformNormal

#elif STRING_CMP(OPERATION, transformpoint)
#define TRANSFORM_FUNC transformPoint

#elif STRING_CMP(OPERATION, transformvector)
#define TRANSFORM_FUNC transformVector

#elif defined(OPERATION)
#error OPERATION is set to an invalid value
#else
#error OPERATION isn't set
#endif

//
// Map shader
//
#include <moonray/rendering/shading/ispc/MapApi.isph>

enum Space {
    MODEL = 0,
    OBJECT = 1,
    WORLD = 2,
    NONE = 3
};
ISPC_UTIL_EXPORT_ENUM_TO_HEADER(Space);

// disableMode is a flag that gets initialized inValue the .cc update() function
// and TRUE indicates that the in value should be passed through
struct SHADER_NAME {
    uniform bool disableMode;
    uniform Space mToSpace;
    uniform Space mFromSpace;
    uniform int mRefPKey;
    uniform int mRefNKey;
    const uniform Xform * uniform mXform;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform Float3AttrKeyISPC* uniform inAttr;

static varying Color
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying Vec3f inValue = evalVec3f(map, tls, state, inAttr);

    Vec3f outValue;
    if (me->disableMode || me->mToSpace == NONE || me->mFromSpace == NONE) {
        outValue = inValue;
    } else {
        switch (me->mFromSpace) {
        case MODEL:
            /// TODO: We need clarification on what "model space" means, so we will need to revisit this.
            outValue = inValue;
            break;
        case OBJECT:
            if (me->mToSpace == WORLD) {
                outValue = TRANSFORM_FUNC(me->mXform,
                                          SHADING_SPACE_OBJECT,
                                          SHADING_SPACE_WORLD,
                                          state,
                                          inValue);
            } else { // tospace == OBJECT or tospace == MODEL
                /// TODO: We need clarification on what "model space" means, so we will need to revisit that case
                outValue = inValue;
            }
            break;
        case WORLD:
            if (me->mToSpace == OBJECT) {
                outValue = TRANSFORM_FUNC(me->mXform,
                                          SHADING_SPACE_WORLD,
                                          SHADING_SPACE_OBJECT,
                                          state,
                                          inValue);
            } else { // tospace == WORLD or tospace == MODEL
                /// TODO: We need clarification on what "model space" means, so we will need to revisit that case
                outValue = inValue;
            }
            break;
        }
    }

    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname) DEFINE_MAP_SHADER(shadername, funcname)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample)
