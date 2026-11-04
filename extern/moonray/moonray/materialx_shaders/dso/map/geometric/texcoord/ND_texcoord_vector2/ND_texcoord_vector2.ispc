// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_texcoord_vector2.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_texcoord_vector2 {
    uniform bool disableMode;
    uniform int mIndex;
    uniform int mStAttributeIndex;
    uniform int mUvAttributeIndex;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_texcoord_vector2);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_texcoord_vector2* uniform me = MAP_GET_ISPC_CPTR(ND_texcoord_vector2, map);

    // SdfType is float2
    // NdrType is float[]
    varying Vec2f outValue;
    if (me->disableMode) {
        outValue = Vec2f_ctor(0.0f, 0.0f);
    } else {
        const int stKey = me->mStAttributeIndex;
        const int uvKey = me->mUvAttributeIndex;

        // Check for "st" attribute first
        if (isProvided(state, stKey)) {
            outValue = getVec2fAttribute(tls, state, stKey);
        }
        // If "st" not available, check for "uv" attribute
        else if (isProvided(state, uvKey)) {
            outValue = getVec2fAttribute(tls, state, uvKey);
        }
        // Fallback to 2D texture coordinates
        else {
            outValue = state.mSt;
        }
    }
    return Color_ctor(outValue.x, -outValue.y, 0.0f);
}

DEFINE_MAP_SHADER(ND_texcoord_vector2, sample)

