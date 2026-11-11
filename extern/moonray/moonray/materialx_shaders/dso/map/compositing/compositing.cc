// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file compositing.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// supports the following compositing operations with fg, bg, and mix inputs
// use #define OPERATION before including this file
// burn, dodge, screen, difference, plus, minus, overlay, inside, outside,
// in, mask, matte, out, over, disjointover, premult, unpremult, mix
//
// supports the following variants:
// use #define VARIANT before including this file
// float, color3, color4, color3_color3, color4_color4,
// vector2, vector2_vector2, vector3, vector3_vector3, vector4, vector4_vector4
//
// Not every combination of OPERATION and VARIANT is a standard MaterialX shader
// See README for list of shaders.

// OPERATIONS
#define UNIQUE_ID_burn         300
#define UNIQUE_ID_dodge        301
#define UNIQUE_ID_screen       302
#define UNIQUE_ID_difference   303
#define UNIQUE_ID_plus         304
#define UNIQUE_ID_minus        305
#define UNIQUE_ID_overlay      306
#define UNIQUE_ID_inside       307
#define UNIQUE_ID_outside      308
#define UNIQUE_ID_in           309
#define UNIQUE_ID_mask         310
#define UNIQUE_ID_matte        311
#define UNIQUE_ID_out          312
#define UNIQUE_ID_over         313
#define UNIQUE_ID_disjointover 314
#define UNIQUE_ID_premult      315
#define UNIQUE_ID_unpremult    316
#define UNIQUE_ID_mix          317

// All compositing nodes have the same defaults
#define IN_DEFAULT_FLOAT 0.0
#define MIX_DEFAULT_FLOAT 1.0

//
// Configure the variants, starting with FG and BG
// Output type always matches BG
//
#if STRING_CMP(VARIANT,float)

#define IN_TYPE Float
#define IN_DEFAULT IN_DEFAULT_FLOAT
#define MIX_TYPE Float
#define MIX_DEFAULT MIX_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color3)

#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Float
#define MIX_DEFAULT MIX_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4)

#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Float
#define MIX_DEFAULT MIX_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,color3_color3)

#define IN_TYPE Color
#define IN_DEFAULT Color(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Color
#define MIX_DEFAULT Color(MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,color4_color4)

#define IN_TYPE Rgba
#define IN_DEFAULT Rgba(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Rgba
#define MIX_DEFAULT Rgba(MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGBA

#elif STRING_CMP(VARIANT,vector2)

#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Float
#define MIX_DEFAULT MIX_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector2_vector2)

#define IN_TYPE Vec2f
#define IN_DEFAULT Vec2f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Vec2f
#define MIX_DEFAULT Vec2f(MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3)

#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Float
#define MIX_DEFAULT MIX_DEFAULT_FLOAT
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector3_vector3)

#define IN_TYPE Vec3f
#define IN_DEFAULT Vec3f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Vec3f
#define MIX_DEFAULT Vec3f(MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_RGB

#elif STRING_CMP(VARIANT,vector4)

#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Float
#define MIX_DEFAULT MIX_DEFAULT_FLOAT
#define OUT_TYPE TYPE_VEC4F

#elif STRING_CMP(VARIANT,vector4_vector4)

#define IN_TYPE Vec4f
#define IN_DEFAULT Vec4f(IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT,IN_DEFAULT_FLOAT)
#define MIX_TYPE Rgba
#define MIX_DEFAULT Vec4f(MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT,MIX_DEFAULT_FLOAT)
#define OUT_TYPE TYPE_VEC4F

#elif defined(VARIANT)
#error VARIANT isn't set or is set to an invalid value
#else
#error VARIANT isn't set or is set to an invalid value
#endif

#define IN_EVAL CONCAT2(eval,IN_TYPE)
#define MIX_EVAL CONCAT2(eval,MIX_TYPE)

#include <moonray/rendering/shading/MapApi.h>
//
// Deal with multiple output types mapping to Color
//
#if STRING_CMP(IN_TYPE,Float) || STRING_CMP(IN_TYPE,Color) || STRING_CMP(IN_TYPE,Vec2f) || STRING_CMP(IN_TYPE,Vec3f)
#define RETURN_TYPE Color
#else
#define RETURN_TYPE IN_TYPE
#endif


//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
#if STRING_CMP(OPERATION,inside) || STRING_CMP(OPERATION,outside)
    // inside/outside use 'in' and 'mask' parameters
    AttributeKey<IN_TYPE> inAttr;
    AttributeKey<IN_TYPE> maskAttr;
#elif STRING_CMP(OPERATION,premult) || STRING_CMP(OPERATION,unpremult)
    // premult/unpremult use single 'in' parameter
    AttributeKey<IN_TYPE> inAttr;
#else
    // All other operations use 'fg', 'bg', and 'mix'
    AttributeKey<IN_TYPE> fgAttr;
    AttributeKey<IN_TYPE> bgAttr;
    AttributeKey<MIX_TYPE> mixAttr;
#endif
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
#if STRING_CMP(OPERATION,inside) || STRING_CMP(OPERATION,outside)
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    maskAttr = sceneClass.declareAttribute<IN_TYPE>("mask", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
#elif STRING_CMP(OPERATION,premult) || STRING_CMP(OPERATION,unpremult)
    inAttr = sceneClass.declareAttribute<IN_TYPE>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
#else
    fgAttr = sceneClass.declareAttribute<IN_TYPE>("fg", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    bgAttr = sceneClass.declareAttribute<IN_TYPE>("bg", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    mixAttr = sceneClass.declareAttribute<MIX_TYPE>("mix", MIX_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
#endif
MTLX_ATTR_DEFINE_END

//
// end of attributes.cc
//

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map* self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, RETURN_TYPE *sample);
    ispc::SHADER_NAME mIspc;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass,
                         const std::string& name) :
    Parent(sceneClass, name)
{
    // the functions pointers are unions so it doesn't
    // really matter which one is assigned to and the variability
    // is easier to manage with macros with picking one and forcing
    // with a cast
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = OUT_TYPE;
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
}

float
doOperation(float fg, float bg)
{
#if STRING_CMP(OPERATION,burn)
    return fg < sEpsilon ? 0.0f : 1.0f - (1.0f - bg)/fg;
#elif STRING_CMP(OPERATION,dodge)
    return scene_rdl2::math::abs(1.0f - fg) < sEpsilon ? 0.0f : bg/(1.0f - fg);
#elif STRING_CMP(OPERATION,screen)
    return 1.0f - (1.0f - fg) * (1.0f - bg);
#elif STRING_CMP(OPERATION,difference)
    return scene_rdl2::math::abs(fg - bg);
#elif STRING_CMP(OPERATION,plus)
    return fg + bg;
#elif STRING_CMP(OPERATION,minus)
    return bg - fg;
#elif STRING_CMP(OPERATION,overlay)
    return bg < 0.5f ? 2.0f * fg * bg : 1.0f - 2.0f * (1.0f - fg) * (1.0f - bg);
#elif STRING_CMP(OPERATION,inside)
    return fg * bg;  // inside uses mask param, but for compositing it's fg*bg
#elif STRING_CMP(OPERATION,outside)
    return fg * (1.0f - bg);  // outside uses mask param
#elif STRING_CMP(OPERATION,in)
    return fg * bg;  // in uses alpha masking
#elif STRING_CMP(OPERATION,mask)
    return fg * bg;  // mask is same as in
#elif STRING_CMP(OPERATION,matte)
    return fg + bg * (1.0f - fg);  // matte composite
#elif STRING_CMP(OPERATION,out)
    return fg * (1.0f - bg);  // out uses alpha masking
#elif STRING_CMP(OPERATION,over)
    return fg + bg * (1.0f - fg);  // over composite
#elif STRING_CMP(OPERATION,disjointover)
    return fg + bg;  // disjoint over (simplified)
#elif STRING_CMP(OPERATION,premult)
    return fg * bg;  // premultiply by alpha
#elif STRING_CMP(OPERATION,unpremult)
    return bg < sEpsilon ? 0.0f : fg / bg;  // unpremultiply by alpha
#elif STRING_CMP(OPERATION,mix)
    return fg;  // mix just returns fg (mix factor handled separately)
#elif defined(OPERATION)
#error OPERATION is set to an invalid value
#else
#error OPERATION isn't set
#endif
}

void
SHADER_NAME::sample(const Map* self,
                          moonray::shading::TLState *tls,
                    const moonray::shading::State& state,
                    RETURN_TYPE* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    IN_TYPE fgValue;
    IN_TYPE bgValue;
    MIX_TYPE mixValue;

#if STRING_CMP(OPERATION,inside) || STRING_CMP(OPERATION,outside)
    // inside/outside use 'in' and 'mask' parameters
    fgValue = IN_EVAL(me, inAttr, tls, state);
    bgValue = IN_EVAL(me, maskAttr, tls, state);
    mixValue = 1.0f;  // No mix parameter for inside/outside
#elif STRING_CMP(OPERATION,premult) || STRING_CMP(OPERATION,unpremult)
    // premult/unpremult use single 'in' parameter
    fgValue = IN_EVAL(me, inAttr, tls, state);
    bgValue = IN_TYPE(0);  // Not used
    mixValue = 1.0f;  // No mix parameter for premult/unpremult
#else
    // All other operations use 'fg', 'bg', and 'mix'
    fgValue = IN_EVAL(me, fgAttr, tls, state);
    bgValue = IN_EVAL(me, bgAttr, tls, state);
    mixValue = MIX_EVAL(me, mixAttr, tls, state);
#endif

    IN_TYPE outValue;
    if (me->mIspc.disableMode) {
#if STRING_CMP(OPERATION,premult) || STRING_CMP(OPERATION,unpremult)
        outValue = fgValue;  // For premult/unpremult, disable returns input
#else
        outValue = bgValue;
#endif
    } else {
#if STRING_CMP(VARIANT,float)
        float tmp = doOperation(fgValue, bgValue);
        outValue = lerp(bgValue, tmp, mixValue);
        // mimic Karma behavior by clamping negative values post-lerp
        outValue = max(0.0f, outValue);
#elif STRING_CMP(VARIANT,color3)
        Color tmp;
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        outValue = lerp(bgValue, tmp, mixValue);
        // mimic Karma behavior by clamping negative values post-lerp
        outValue.r = max(0.0f, outValue.r);
        outValue.g = max(0.0f, outValue.g);
        outValue.b = max(0.0f, outValue.b);
#elif STRING_CMP(VARIANT,color3_color3)
        Color tmp;
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        outValue.r = lerp(bgValue.r, tmp.r, mixValue.r);
        outValue.g = lerp(bgValue.g, tmp.g, mixValue.g);
        outValue.b = lerp(bgValue.b, tmp.b, mixValue.b);
        // mimic Karma behavior by clamping negative values post-lerp
        outValue.r = max(0.0f, outValue.r);
        outValue.g = max(0.0f, outValue.g);
        outValue.b = max(0.0f, outValue.b);
#elif STRING_CMP(VARIANT,color4)
        // Color4 operations have different behaviors depending on the operation type
        Rgba tmp;

        // Operations with alpha compositing (use whole color4 with alpha as scalar):
        // in, mask, out, over
#if STRING_CMP(OPERATION,in)
        tmp.r = fgValue.r * bgValue.a;
        tmp.g = fgValue.g * bgValue.a;
        tmp.b = fgValue.b * bgValue.a;
        tmp.a = fgValue.a * bgValue.a;
        outValue = lerp(bgValue, tmp, mixValue);

#elif STRING_CMP(OPERATION,mask)
        tmp.r = bgValue.r * fgValue.a;
        tmp.g = bgValue.g * fgValue.a;
        tmp.b = bgValue.b * fgValue.a;
        tmp.a = fgValue.a * bgValue.a;
        outValue = lerp(bgValue, tmp, mixValue);

#elif STRING_CMP(OPERATION,out)
        // Reference: (fg * (1-bg.a) * mix) + (bg * (1-mix))
        tmp = fgValue * (1.0f - bgValue.a);
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,over)
        // Reference: (fg + bg*(1-fg.a)) * mix + bg * (1-mix)
        tmp = fgValue + bgValue * (1.0f - fgValue.a);
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,matte)
        // Reference: color4(fg.rgb*fg.a + bg.rgb*(1-fg.a), fg.a + bg.a*(1-fg.a)) * mix + bg * (1-mix)
        tmp.r = fgValue.r * fgValue.a + bgValue.r * (1.0f - fgValue.a);
        tmp.g = fgValue.g * fgValue.a + bgValue.g * (1.0f - fgValue.a);
        tmp.b = fgValue.b * fgValue.a + bgValue.b * (1.0f - fgValue.a);
        tmp.a = fgValue.a + bgValue.a * (1.0f - fgValue.a);
        outValue = tmp * mixValue + bgValue * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,disjointover)
        // Reference: complex conditional based on summed alpha
        float summedAlpha = fgValue.a + bgValue.a;
        if (summedAlpha <= 1.0f) {
            tmp.r = fgValue.r + bgValue.r;
            tmp.g = fgValue.g + bgValue.g;
            tmp.b = fgValue.b + bgValue.b;
        } else {
            if (fabsf(bgValue.a) < sEpsilon) {
                tmp.r = 0.0f;
                tmp.g = 0.0f;
                tmp.b = 0.0f;
            } else {
                float x = (1.0f - fgValue.a) / bgValue.a;
                tmp.r = fgValue.r + bgValue.r * x;
                tmp.g = fgValue.g + bgValue.g * x;
                tmp.b = fgValue.b + bgValue.b * x;
            }
        }
        tmp.a = min(summedAlpha, 1.0f);
        outValue.r = tmp.r * mixValue + bgValue.r * (1.0f - mixValue);
        outValue.g = tmp.g * mixValue + bgValue.g * (1.0f - mixValue);
        outValue.b = tmp.b * mixValue + bgValue.b * (1.0f - mixValue);
        outValue.a = tmp.a * mixValue + bgValue.a * (1.0f - mixValue);

#elif STRING_CMP(OPERATION,premult)
        // Reference: color4(fg.rgb * fg.a, fg.a) - no mix parameter in reference!
        outValue.r = fgValue.r * fgValue.a;
        outValue.g = fgValue.g * fgValue.a;
        outValue.b = fgValue.b * fgValue.a;
        outValue.a = fgValue.a;

#elif STRING_CMP(OPERATION,unpremult)
        // Reference: color4(fg.rgb / fg.a, fg.a) - no mix parameter in reference!
        if (fabsf(fgValue.a) < sEpsilon) {
            outValue.r = 0.0f;
            outValue.g = 0.0f;
            outValue.b = 0.0f;
        } else {
            outValue.r = fgValue.r / fgValue.a;
            outValue.g = fgValue.g / fgValue.a;
            outValue.b = fgValue.b / fgValue.a;
        }
        outValue.a = fgValue.a;

#elif STRING_CMP(OPERATION,inside)
        // Reference: in * mask (component-wise multiply)
        tmp.r = fgValue.r * bgValue.r;
        tmp.g = fgValue.g * bgValue.g;
        tmp.b = fgValue.b * bgValue.b;
        tmp.a = fgValue.a * bgValue.a;
        outValue = lerp(bgValue, tmp, mixValue);

#elif STRING_CMP(OPERATION,outside)
        // Reference: in * (1 - mask) (component-wise)
        tmp.r = fgValue.r * (1.0f - bgValue.r);
        tmp.g = fgValue.g * (1.0f - bgValue.g);
        tmp.b = fgValue.b * (1.0f - bgValue.b);
        tmp.a = fgValue.a * (1.0f - bgValue.a);
        outValue = lerp(bgValue, tmp, mixValue);

#else
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        tmp.a = doOperation(fgValue.a, bgValue.a);
        outValue = lerp(bgValue, tmp, mixValue);
#endif
        // mimic Karma behavior by clamping negative values post-lerp (except for premult/unpremult which don't use lerp)
#if !STRING_CMP(OPERATION,premult) && !STRING_CMP(OPERATION,unpremult)
        outValue.r = max(0.0f, outValue.r);
        outValue.g = max(0.0f, outValue.g);
        outValue.b = max(0.0f, outValue.b);
        outValue.a = max(0.0f, outValue.a);
#endif

#elif STRING_CMP(VARIANT,color4_color4)
        Rgba tmp;
        tmp.r = doOperation(fgValue.r, bgValue.r);
        tmp.g = doOperation(fgValue.g, bgValue.g);
        tmp.b = doOperation(fgValue.b, bgValue.b);
        tmp.a = doOperation(fgValue.a, bgValue.a);
        outValue.r = max(0.0f, lerp(bgValue.r, tmp.r, mixValue.r));
        outValue.g = max(0.0f, lerp(bgValue.g, tmp.g, mixValue.g));
        outValue.b = max(0.0f, lerp(bgValue.b, tmp.b, mixValue.b));
        outValue.a = max(0.0f, lerp(bgValue.a, tmp.a, mixValue.a));
#elif STRING_CMP(VARIANT,vector2)
        Vec2f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        outValue = lerp(bgValue, tmp, mixValue);
#elif STRING_CMP(VARIANT,vector2_vector2)
        Vec2f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        outValue.x = lerp(bgValue.x, tmp.x, mixValue);
        outValue.y = lerp(bgValue.y, tmp.y, mixValue);
#elif STRING_CMP(VARIANT,vector3)
        Vec3f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        outValue = lerp(bgValue, tmp, mixValue);
#elif STRING_CMP(VARIANT,vector3_vector3)
        Vec3f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        outValue.x = lerp(bgValue.x, tmp.x, mixValue);
        outValue.y = lerp(bgValue.y, tmp.y, mixValue);
        outValue.z = lerp(bgValue.z, tmp.z, mixValue);
#elif STRING_CMP(VARIANT,vector4)
        Vec4f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        tmp.w = doOperation(fgValue.w, bgValue.w);
        outValue = lerp(bgValue, tmp, mixValue);
#elif STRING_CMP(VARIANT,vector4_vector4)
        Vec4f tmp;
        tmp.x = doOperation(fgValue.x, bgValue.x);
        tmp.y = doOperation(fgValue.y, bgValue.y);
        tmp.z = doOperation(fgValue.z, bgValue.z);
        tmp.w = doOperation(fgValue.w, bgValue.w);
        outValue.x = lerp(bgValue.x, tmp.x, mixValue);
        outValue.y = lerp(bgValue.y, tmp.y, mixValue);
        outValue.z = lerp(bgValue.z, tmp.z, mixValue);
        outValue.w = lerp(bgValue.w, tmp.w, mixValue);
#endif
    }

    //
    // Convert the output to the return type if necessary.
    //
#if STRING_CMP(IN_TYPE,Float)
    *sample = Color(outValue, outValue, outValue);
#elif STRING_CMP(IN_TYPE,Vec2f)
    *sample = Color(outValue.x, outValue.y, 0.0);
#elif STRING_CMP(IN_TYPE,Vec3f)
    *sample = Color(outValue.x, outValue.y, outValue.z);
#else
    *sample = outValue;
#endif
}
