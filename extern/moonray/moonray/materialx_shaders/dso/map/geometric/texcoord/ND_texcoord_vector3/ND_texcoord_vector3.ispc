// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_texcoord_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_texcoord_vector3 {
    uniform bool disableMode;
    const uniform Xform * uniform mXform;
    uniform int mIndex;
    uniform int mStAttributeIndex;
    uniform int mUvAttributeIndex;
    uniform int mMissingStEvent;
    uniform int mMissingUvEvent;
    uniform int mMissingStUvEvent;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_texcoord_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_texcoord_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_texcoord_vector3, map);

    varying Vec3f outValue;
    if (me->disableMode) {
        outValue = Vec3f_ctor(0.0f, 0.0f, 0.0f);
    } else {
        const int stKey = me->mStAttributeIndex;
        const int uvKey = me->mUvAttributeIndex;

        // Check for "st" attribute first
        if (isProvided(state, stKey)) {
            outValue = getVec3fAttribute(tls, state, stKey);
        }
        // If "st" not available, check for "uv" attribute
        else if (isProvided(state, uvKey)) {
            outValue = getVec3fAttribute(tls, state, uvKey);
        }
        // Fallback to 2D texture coordinates
        else {
            logEvent(map, me->mMissingStUvEvent);
            const varying Vec2f st = getSt(state);
            outValue = Vec3f_ctor(st.x, st.y, 0.0f);
        }
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_texcoord_vector3, sample)

