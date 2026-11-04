// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_texcoord_vector3.cc

#include "attributes.cc"
#include "ND_texcoord_vector3_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_texcoord_vector3, Map)
public:
    ND_texcoord_vector3(SceneClass const &sceneClass, std::string const &name);
    void update() override;

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);

    void createMissingStUvLogEvent();

    ispc::ND_texcoord_vector3 mIspc;

    std::unique_ptr<moonray::shading::Xform> mXform;

RDL2_DSO_CLASS_END(ND_texcoord_vector3)

ND_texcoord_vector3::ND_texcoord_vector3(const SceneClass& sceneClass,
        const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_texcoord_vector3::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_texcoord_vector3_getSampleFunc();
}

void
ND_texcoord_vector3::createMissingStUvLogEvent()
{
    mIspc.mMissingStUvEvent = sLogEventRegistry.createEvent(scene_rdl2::logging::WARN_LEVEL,
                                                            "Missing primitive attributes 'st' and 'uv', falling back to 2D texture coordinates");
}

void
ND_texcoord_vector3::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));

    mXform = std::make_unique<moonray::shading::Xform>(this);
    mIspc.mXform = mXform->getIspcXform();

    int index = scene_rdl2::math::max(0, get(indexAttr));
    mIspc.mIndex = index;

    // Create attribute keys for "st" and "uv" with optional index suffix
    std::string stName = "st";
    std::string uvName = "uv";
    
    // If index is greater than 0, append suffix
    if (index > 0) {
        stName += std::to_string(index);
        uvName += std::to_string(index);
    }
    
    moonray::shading::TypedAttributeKey<Vec3f> stAttributeKey(stName);
    moonray::shading::TypedAttributeKey<Vec3f> uvAttributeKey(uvName);
    
    mIspc.mStAttributeIndex = stAttributeKey;
    mIspc.mUvAttributeIndex = uvAttributeKey;
    
    mOptionalAttributes.push_back(stAttributeKey);
    mOptionalAttributes.push_back(uvAttributeKey);

    // Create log event
    createMissingStUvLogEvent();
}

void
ND_texcoord_vector3::sample(const Map* self, moonray::shading::TLState *tls,
                            const moonray::shading::State& state, Color* sample)
{
    const ND_texcoord_vector3* me = static_cast<const ND_texcoord_vector3*>(self);

    Vec3f outValue;
    if (me->mIspc.disableMode) {
        outValue = Vec3f(0.0f, 0.0f, 0.0f);
    } else {
        int stKey = me->mIspc.mStAttributeIndex;
        int uvKey = me->mIspc.mUvAttributeIndex;

        // Check for "st" attribute first
        if (state.isProvided(stKey)) {
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Vec3f>(stKey));
        }
        // If "st" not available, check for "uv" attribute
        else if (state.isProvided(uvKey)) {
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Vec3f>(uvKey));
        }
        // Fallback to 2D texture coordinates
        else {
            moonray::shading::logEvent(me, me->mIspc.mMissingStUvEvent);
            Vec2f st = state.getSt();
            outValue = Vec3f(st.x, st.y, 0.0f);
        }
    }
    *sample = Color(outValue.x, outValue.y, outValue.z);
}

