// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_position_vector3.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_position_vector3 {
    uniform int mRefPKey;
    const uniform Xform * uniform mXform;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_position_vector3);

static Color
sample(const uniform Map *            uniform  map,
             uniform ShadingTLState * uniform  tls,
       const varying State &                   state)
{
    const uniform ND_position_vector3* uniform me = MAP_GET_ISPC_CPTR(ND_position_vector3, map);
    varying int spaceValue;
    spaceValue = getSpaceAttr(map);

    varying Vec3f outValue;
    switch (spaceValue) {
      // model space
      case 0:
        // using object space for now
        SHADER_DATA_getRefP(me->mRefPKey, tls, state, &outValue);
        break;
      // object space
      case 1:
        // get position (in render space)
        outValue = getP(state);
        // transform position to object space
        outValue = transformPoint(me->mXform,
                                  SHADING_SPACE_RENDER,
                                  SHADING_SPACE_OBJECT,
                                  state,
                                  outValue);
        break;
      // world space
      case 2:
        // get position (in render space)
        outValue = getP(state);
        // transform position to object space
        outValue = transformPoint(me->mXform,
                                  SHADING_SPACE_RENDER,
                                  SHADING_SPACE_WORLD,
                                  state,
                                  outValue);
        break;
    }
    return Color_ctor(outValue.x, outValue.y, outValue.z);
}

DEFINE_MAP_SHADER(ND_position_vector3, sample)

