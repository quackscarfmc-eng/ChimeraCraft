// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "g22_ap1_to_lin_rec709.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>
#include <scene_rdl2/common/math/ispc/ColorSpace.isph>
#include <scene_rdl2/common/math/ispc/Math.isph>

// supports the following variants.
// use #define VARIANT before including this file
// color3, color4

#define OPERATION g22_ap1_to_lin_rec709

//
// Configure the variants
// The output type is always the same as IN_TYPE
//
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



//
// disableMode is a flag that gets initialized in the .cc update() function
// and TRUE indicates that the in value should just be passed through
// unchanged
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;

varying IN_TYPE
doG22Ap1ToLinRec709(varying IN_TYPE inValue)
{
    // Gamma 2.2 AP1 (ACEScg) to Linear Rec.709
    // Step 1: Apply inverse gamma of 2.2 to linearize
    IN_TYPE linearAp1 = inValue;
    linearAp1.r = max(0.0f, inValue.r);
    linearAp1.g = max(0.0f, inValue.g);
    linearAp1.b = max(0.0f, inValue.b);
    
    linearAp1.r = pow(linearAp1.r, 2.2f);
    linearAp1.g = pow(linearAp1.g, 2.2f);
    linearAp1.b = pow(linearAp1.b, 2.2f);
    
    // Step 2: Apply ACEScg (AP1) to Linear Rec.709 matrix
    const uniform float m00 =  1.705050786f;
    const uniform float m01 = -0.621792197f;
    const uniform float m02 = -0.083258589f;
    const uniform float m10 = -0.130256490f;
    const uniform float m11 =  1.140804506f;
    const uniform float m12 = -0.010548016f;
    const uniform float m20 = -0.024003039f;
    const uniform float m21 = -0.128968775f;
    const uniform float m22 =  1.152971814f;

    IN_TYPE outValue = inValue;
    
    varying float r = linearAp1.r;
    varying float g = linearAp1.g;
    varying float b = linearAp1.b;

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
        outValue = doG22Ap1ToLinRec709(inValue);
    }

    return outValue;
}


// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
