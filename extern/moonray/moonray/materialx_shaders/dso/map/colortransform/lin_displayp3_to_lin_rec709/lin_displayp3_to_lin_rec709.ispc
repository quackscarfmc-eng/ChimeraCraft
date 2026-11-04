// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "lin_displayp3_to_lin_rec709.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <scene_rdl2/common/math/ispc/ColorSpace.isph>

#define OPERATION lin_displayp3_to_lin_rec709

#if STRING_CMP(VARIANT,color3)
#define IN_TYPE Color
#define IN_ATTRKEY Float3AttrKeyISPC
#elif STRING_CMP(VARIANT,color4)
#define IN_TYPE Col4f
#define IN_ATTRKEY Float4AttrKeyISPC
#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT is not set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif

struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;

varying IN_TYPE doLinDisplayp3ToLinRec709(varying IN_TYPE inValue)
{
    // Linear Display P3 to Linear Rec.709 transformation matrix
    const uniform float m00 =  1.22493029f;
    const uniform float m01 = -0.04205868f;
    const uniform float m02 = -0.01964128f;
    const uniform float m10 = -0.22492968f;
    const uniform float m11 =  1.04205894f;
    const uniform float m12 = -0.07864794f;
    const uniform float m20 =  0.00000006f;
    const uniform float m21 = -0.00000001f;
    const uniform float m22 =  1.09828925f;

    IN_TYPE outValue = inValue;
    varying float r = inValue.r;
    varying float g = inValue.g;
    varying float b = inValue.b;

    outValue.r = m00 * r + m01 * g + m02 * b;
    outValue.g = m10 * r + m11 * g + m12 * b;
    outValue.b = m20 * r + m21 * g + m22 * b;

    return outValue;
}

varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);
    varying IN_TYPE inValue = IN_EVAL(map, tls, state, inAttr);

    varying IN_TYPE outValue = inValue;
    if (!me->disableMode) {
        outValue = doLinDisplayp3ToLinRec709(inValue);
    }

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
