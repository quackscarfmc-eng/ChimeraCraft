// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_bitangent_vector3.cc

#include "attributes.cc"
#include "ND_bitangent_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_bitangent_vector3, Map)
public:
    ND_bitangent_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_bitangent_vector3 mIspc;

    std::unique_ptr<moonray::shading::Xform> mXform;

RDL2_DSO_CLASS_END(ND_bitangent_vector3)

ND_bitangent_vector3::ND_bitangent_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_bitangent_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_bitangent_vector3_getSampleFunc();
}

void
ND_bitangent_vector3::update()
{
    mXform = std::make_unique<moonray::shading::Xform>(this);
    mIspc.mXform = mXform->getIspcXform();

    // Need the refP key for model space
    mIspc.mRefPKey = moonray::shading::StandardAttributes::sRefP;
    mIspc.mRefNKey = moonray::shading::StandardAttributes::sRefN;

    mRequiredAttributes.clear();
    mOptionalAttributes.clear();

    if (get(spaceAttr) == 0 /* model space */) {
        mRequiredAttributes.push_back(mIspc.mRefPKey);
        mRequiredAttributes.push_back(mIspc.mRefNKey);
    }
}

void
ND_bitangent_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_bitangent_vector3* me = static_cast<const ND_bitangent_vector3*>(self);

    Int spaceValue;
    spaceValue = me->get(spaceAttr);

    // N/A -- We currently only support one set of texture coordinates
    Int indexValue;
    indexValue = evalInt(me, indexAttr, tls, state);

    Vec3f outValue, tangent, normal;
    outValue = Vec3f(0.0f, 0.0f, 0.0f);
    switch (spaceValue) {
        // model space
        case 0:
            // find refdPds
            state.getdVec3fAttrds(moonray::shading::StandardAttributes::sRefP, tangent);

            // get the reference surface normal
            state.getRefN(normal);

            // find the cross product
            outValue = normalize(cross(normal, tangent));

            break;
        // object space
        case 1:
            // get tangent (in render space)
            tangent = state.getdPds();

            // get normal (in render space)
            normal = state.getN();

            // get the cross product
            outValue = normalize(cross(normal, tangent));

            // transform bitangent to object space
            outValue = me->mXform->transformNormal(ispc::SHADING_SPACE_RENDER, 
                                                   ispc::SHADING_SPACE_OBJECT,
                                                   state,
                                                   outValue);
            break;
        // world space
        case 2:
            // get tangent (in render space)
            tangent = state.getdPds();

            // get normal (in render space)
            normal = state.getN();

            // get the cross product
            outValue = normalize(cross(normal, tangent));

            // transform tangent to world space
            outValue = me->mXform->transformNormal(ispc::SHADING_SPACE_RENDER, 
                                                   ispc::SHADING_SPACE_WORLD,
                                                   state,
                                                   outValue);
            break;
    }
    *sample = Color(outValue.x, outValue.y, outValue.z);
}

