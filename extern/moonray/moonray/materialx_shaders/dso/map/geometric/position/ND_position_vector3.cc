// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_position_vector3.cc

#include "attributes.cc"
#include "ND_position_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_position_vector3, Map)
public:
    ND_position_vector3(SceneClass const &sceneClass, std::string const &name);
    void update();

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);
    ispc::ND_position_vector3 mIspc;

    std::unique_ptr<moonray::shading::Xform> mXform;

RDL2_DSO_CLASS_END(ND_position_vector3)

ND_position_vector3::ND_position_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_position_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_position_vector3_getSampleFunc();
}

void
ND_position_vector3::update()
{
    mIspc.mRefPKey = moonray::shading::StandardAttributes::sRefP;

    mXform = std::make_unique<moonray::shading::Xform>(this);
    mIspc.mXform = mXform->getIspcXform();

    mRequiredAttributes.clear();
    mOptionalAttributes.clear();

    if (get(spaceAttr) == 0 /* model space */) {
        mRequiredAttributes.push_back(mIspc.mRefPKey);
    }
}

void
ND_position_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                 const moonray::shading::State& state, Color* sample)
{
    const ND_position_vector3* me = static_cast<const ND_position_vector3*>(self);

    Int spaceValue;
    spaceValue = me->get(spaceAttr);

    Vec3f outValue;
    switch (spaceValue) {
      // model space
      case 0:
        state.getRefP(outValue);
        break;
      // object space
      case 1:
        // get position (in render space)
        outValue = state.getP();
        // transform position to object space
        outValue = me->mXform->transformPoint(ispc::SHADING_SPACE_RENDER, 
                                              ispc::SHADING_SPACE_OBJECT,
                                              state,
                                              outValue);
        break;
      // world space
      case 2:
        outValue = state.getP();

        outValue = me->mXform->transformPoint(ispc::SHADING_SPACE_RENDER, 
                                              ispc::SHADING_SPACE_WORLD,
                                              state,
                                              outValue);
        break;
    }
    *sample = Color(outValue.x, outValue.y, outValue.z);
}

