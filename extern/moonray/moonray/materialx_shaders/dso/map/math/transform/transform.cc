// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file transform.cc

#include <materialx_shaders/map/MtlxVariantTypes.h>

using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

// Supports the following variants/operations
//      - transformnormal_vector3
//      - transformpoint_vector3
//      - transformvector_vector3
// Use #define VARIANT and OPERATION before including this file

// OPERATIONS
// Since we want to use a different OPERATION value per file we need to
// define these here. This allows us to use the STRING_CMP macro.
// Usually, we would only need to define VARIANT values, which are already
// enumerated in MtlxVariantTypes.hh.
// (starting at 200 to avoid conflicting with MtlxVariantTypes)
#define UNIQUE_ID_transformnormal       200
#define UNIQUE_ID_transformvector       201
#define UNIQUE_ID_transformpoint        202

//
// Configure the defaults
//
#if STRING_CMP(OPERATION,transformnormal)
#define IN_DEFAULT Vec3f(0.f, 0.f, 1.f)
#define TRANSFORM_FUNC transformNormal

#elif STRING_CMP(OPERATION, transformpoint)
#define IN_DEFAULT Vec3f(0.f)
#define TRANSFORM_FUNC transformPoint

#elif STRING_CMP(OPERATION, transformvector)
#define IN_DEFAULT Vec3f(0.f)
#define TRANSFORM_FUNC transformVector

#elif defined(OPERATION)
#error OPERATION is set to an invalid value
#else
#error OPERATION isn't set
#endif

//
// Map shader
//
#include <moonray/rendering/shading/MapApi.h>

//
// start of attributes.cc
//
// Not using a separate file because this depends on macros that
// were defined earlier in this file.
//

MTLX_ATTR_DECLARE_BEGIN
    AttributeKey<Bool> disableAttr;
    AttributeKey<Vec3f> inAttr;
    AttributeKey<String> tospaceAttr;
    AttributeKey<String> fromspaceAttr;
MTLX_ATTR_DECLARE_END

MTLX_ATTR_DEFINE_BEGIN
    disableAttr = sceneClass.declareAttribute<Bool>("disable", 0, FLAGS_NONE, INTERFACE_GENERIC, {});
    inAttr = sceneClass.declareAttribute<Vec3f>("in", IN_DEFAULT, FLAGS_BINDABLE, INTERFACE_GENERIC, {});
    tospaceAttr = sceneClass.declareAttribute<String>("tospace", "", FLAGS_NONE, INTERFACE_GENERIC, {});
    fromspaceAttr = sceneClass.declareAttribute<String>("fromspace", "", FLAGS_NONE, INTERFACE_GENERIC, {});
MTLX_ATTR_DEFINE_END


//
// end of attributes.cc
//

RDL2_DSO_CLASS_BEGIN(SHADER_NAME, Map)

public:
    SHADER_NAME(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::SHADER_NAME mIspc;

    std::unique_ptr<moonray::shading::Xform> mXform;

RDL2_DSO_CLASS_END(SHADER_NAME)

SHADER_NAME::SHADER_NAME(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // it's a Map shader
    mSampleFunc = (SampleFunc) SHADER_NAME::sample;
    mSampleFuncv = (SampleFuncv) ispc::GETSAMPLEFUNC();
    mOutputType = TYPE_RGB;
}

ispc::Space 
getSpace(const String& space)
{
    if      (space == "model")  { return ispc::MODEL; }
    else if (space == "object") { return ispc::OBJECT; }
    else if (space == "world")  { return ispc::WORLD; }
    else                        { return ispc::NONE; }
}

void
SHADER_NAME::update()
{
    mIspc.disableMode = get(disableAttr);

    mXform = std::make_unique<moonray::shading::Xform>(this);
    mIspc.mXform = mXform->getIspcXform();

    mIspc.mRefPKey = moonray::shading::StandardAttributes::sRefP;
    mIspc.mRefNKey = moonray::shading::StandardAttributes::sRefN;

    mRequiredAttributes.clear();
    mOptionalAttributes.clear();

    mIspc.mToSpace = getSpace(get(tospaceAttr));
    mIspc.mFromSpace = getSpace(get(fromspaceAttr));

    if (mIspc.mToSpace == ispc::MODEL || mIspc.mFromSpace == ispc::MODEL) {
        mRequiredAttributes.push_back(mIspc.mRefPKey);
        mRequiredAttributes.push_back(mIspc.mRefNKey);
    }
}

void
SHADER_NAME::sample(const Map* self, moonray::shading::TLState *tls,
                    const moonray::shading::State& state, Color* sample)
{
    const SHADER_NAME* me = static_cast<const SHADER_NAME*>(self);

    const Vec3f inValue = evalVec3f(me, inAttr, tls, state);

    Vec3f outValue;
    if (me->mIspc.disableMode || me->mIspc.mToSpace == ispc::NONE || me->mIspc.mFromSpace == ispc::NONE) {
        outValue = inValue;
    } else {
        switch (me->mIspc.mFromSpace) {
            case ispc::MODEL:
                /// TODO: We need clarification on what "model space" means, so we will need to revisit this.
                outValue = inValue;
                break;

            case ispc::OBJECT:
                // object --> world
                if (me->mIspc.mToSpace == ispc::WORLD){
                    outValue = me->mXform->TRANSFORM_FUNC(ispc::SHADING_SPACE_OBJECT,
                                                          ispc::SHADING_SPACE_WORLD,
                                                          state,
                                                          inValue);
                } else {
                    /// TODO: model space handling
                    // if tospace == OBJECT or NONE, no transform needed
                    outValue = inValue;
                }
                break;

            case ispc::WORLD:
                // world --> object
                if (me->mIspc.mToSpace == ispc::OBJECT) {
                    outValue = me->mXform->TRANSFORM_FUNC(ispc::SHADING_SPACE_WORLD,
                                                          ispc::SHADING_SPACE_OBJECT,
                                                          state,
                                                          inValue);
                } else {
                    /// TODO: model space handling
                    // if tospace == WORLD or NONE, no transform needed
                    outValue = inValue;
                }
                break;
        }
    }

    *sample = Color(outValue.x, outValue.y, outValue.z);
}

