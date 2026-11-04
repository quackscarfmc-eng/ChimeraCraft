// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_bitangent_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_bitangent_vector3 {
    uniform int mRefPKey;
    uniform int mRefNKey;
    const uniform Xform * uniform mXform;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_bitangent_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_bitangent_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_bitangent_vector3, map);

    uniform int spaceValue;
    spaceValue = getSpaceAttr(map);

    // N/A -- We currently only support one set of texture coordinates
    varying int indexValue;
    indexValue = evalIndexAttr(map, tls, state);

    varying Vec3f outValue = Vec3f_ctor(0.0f, 0.0f, 0.0f);
    varying Vec3f tangent, normal;
    switch (spaceValue) {
        // model space
        case 0:
            // find refdPds
            tangent = getdVec3fAttributeds(tls, state, me->mRefPKey);

            // get the reference surface normal
            SHADER_DATA_getRefN(me->mRefPKey, me->mRefNKey, tls, state, &normal);

            // find the cross product
            outValue = normalize(cross(normal, tangent));
            break;
        // object space
        case 1:
            // get tangent (in render space)
            tangent = getdPds(state);

            // get normal (in render space)
            normal = getN(state);

            // get the cross product
            outValue = normalize(cross(normal, tangent));

            // transform bitangent to object space
            outValue = transformNormal(me->mXform,
                                       SHADING_SPACE_RENDER,
                                       SHADING_SPACE_OBJECT,
                                       state,
                                       outValue);
            break;
        // world space
        case 2:
            // get tangent (in render space)
            tangent = getdPds(state);

            // get normal (in render space)
            normal = getN(state);

            // get the cross product
            outValue = normalize(cross(normal, tangent));

            // transform bitangent to world space
            outValue = transformNormal(me->mXform,
                                       SHADING_SPACE_RENDER,
                                       SHADING_SPACE_WORLD,
                                       state,
                                       outValue);
            break;
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_bitangent_vector3, sample)

