// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_normal_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_normal_vector3 {
    uniform int mRefPKey;
    uniform int mRefNKey;
    const uniform Xform * uniform mXform;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_normal_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_normal_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_normal_vector3, map);

    uniform int spaceValue;
    spaceValue = getSpaceAttr(map);

    varying Vec3f outValue = Vec3f_ctor(0.0f, 0.0f, 0.0f);
    switch (spaceValue) {
        // model space
        case 0:
            // get the reference surface normal
            SHADER_DATA_getRefN(me->mRefPKey, me->mRefNKey, tls, state, &outValue);
            outValue = normalize(outValue);
            break;
        // object space
        case 1:
            // get normal (in render space)
            outValue = normalize(getN(state));
            // transform normal to object space
            outValue = transformNormal(me->mXform,
                                       SHADING_SPACE_RENDER,
                                       SHADING_SPACE_OBJECT,
                                       state,
                                       outValue);
            break;
        // world space
        case 2:
            // get normal (in render space)
            outValue = normalize(getN(state));
            // transform normal to world space
            outValue = transformNormal(me->mXform,
                                       SHADING_SPACE_RENDER,
                                       SHADING_SPACE_WORLD,
                                       state,
                                       outValue);
            break;
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_normal_vector3, sample)

