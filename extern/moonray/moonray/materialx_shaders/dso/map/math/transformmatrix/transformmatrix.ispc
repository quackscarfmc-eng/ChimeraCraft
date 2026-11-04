// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "transformmatrix.ispc"

#include <materialx_shaders/map/ispc/MtlxVariantTypes.isph>

// Supports the following variants
//      - vector2M3
//      - vector3
//      - vector3M4
//      - vector4
// Use #define VARIANT before including this file

#define OPERATION transformmatrix

//
// Configure the variants
// The output type is always the same as IN_TYPE
//

#if STRING_CMP(VARIANT,vector2M3)
#define IN_TYPE Vec2f
#define IN_ATTRKEY Float2AttrKeyISPC
#define MAT_TYPE Mat3f
#define MAT_ATTRKEY Mat3fAttrKeyISPC

#elif STRING_CMP(VARIANT,vector3)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define MAT_TYPE Mat3f
#define MAT_ATTRKEY Mat3fAttrKeyISPC

#elif STRING_CMP(VARIANT,vector3M4)
#define IN_TYPE Vec3f
#define IN_ATTRKEY Float3AttrKeyISPC
#define MAT_TYPE Mat4f
#define MAT_ATTRKEY Mat4fAttrKeyISPC

#elif STRING_CMP(VARIANT,vector4)
#define IN_TYPE Vec4f
#define IN_ATTRKEY Float4AttrKeyISPC
#define MAT_TYPE Mat4f
#define MAT_ATTRKEY Mat4fAttrKeyISPC

#elif defined(VARIANT)
#error VARIANT is set to an invalid value
#else
#error VARIANT isn't set
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)
#define MAT_EVAL CONCAT2(eval,MAT_TYPE)

#include <moonray/rendering/shading/ispc/MapApi.isph>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f) || STRING_CMP(IN_TYPE,Color)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif

// disableMode is a flag that gets initialized inValue the .cc update() function
// and TRUE indicates that the in value should be passed through
struct SHADER_NAME {
    uniform bool disableMode;
};
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(SHADER_NAME);

extern uniform IN_ATTRKEY* uniform inAttr;
extern uniform MAT_ATTRKEY* uniform matAttr;

varying Vec3f asVec3f(const varying Vec2f& v) { return Vec3f_ctor(v.x, v.y, 0.0); }
varying Vec2f asVec2f(const varying Vec3f& v) { return Vec2f_ctor(v.x, v.y); }
varying Vec3f asVec3f(const varying Vec4f& v) { return Vec3f_ctor(v.x, v.y, v.z); }
varying Vec4f asVec4f(const varying Vec3f& v) { return Vec4f_ctor(v.x, v.y, v.z, 0.0); }

varying Mat4f asMat4f(const varying Mat3f& m)
{
    varying Vec4f vx = Vec4f_ctor(m.vx.x, m.vx.y, m.vx.z, 0.0);
    varying Vec4f vy = Vec4f_ctor(m.vy.x, m.vy.y, m.vy.z, 0.0);
    varying Vec4f vz = Vec4f_ctor(m.vz.x, m.vz.y, m.vz.z, 0.0);
    varying Vec4f vw = Vec4f_ctor(0.0, 0.0, 0.0, 0.0);
    return Mat4f_ctor(vx, vy, vz, vw);
}

static varying RETURN_TYPE
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform SHADER_NAME* uniform me = MAP_GET_ISPC_CPTR(SHADER_NAME, map);

    varying IN_TYPE inValue = IN_EVAL(map, tls, state, inAttr);
    varying MAT_TYPE matValue = MAT_EVAL(map, tls, state, matAttr);

    varying IN_TYPE outValue;
    if (me->disableMode) {
        outValue = inValue;
    } else {
#if STRING_CMP(VARIANT,vector2M3)
        outValue = asVec2f(transformVector(asMat4f(matValue), asVec3f(inValue)));
#elif STRING_CMP(VARIANT,vector3)
        outValue = transformVector(asMat4f(matValue), inValue);
#elif STRING_CMP(VARIANT,vector3M4)
        outValue = transformVector(matValue, inValue);
#elif STRING_CMP(VARIANT,vector4)
        outValue = inValue * matValue;
#endif
    }

    //
    // Convert to the output type if necessary
    //
#if STRING_CMP(IN_TYPE,Float)
    return Color_ctor(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    return Color_ctor(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    return Color_ctor(outValue.x, outValue.y, outValue.z);
#else
    return outValue;
#endif
}

// The DEFINE_* macros need to be wrapped with an _EVALUATOR layer because they don't handle
// getting a macro as an argument
#define DEFINE_MAP_EVALUATOR(shadername, funcname, type) DEFINE_MAP_SHADER_WITH_TYPE(shadername, funcname, type)
DEFINE_MAP_EVALUATOR(SHADER_NAME, sample, RETURN_TYPE)
