// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file ND_texcoord_vector2.cc

#include "attributes.cc"
#include "ND_texcoord_vector2_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
using namespace scene_rdl2::math;
using namespace scene_rdl2::rdl2;

RDL2_DSO_CLASS_BEGIN(ND_texcoord_vector2, Map)
public:
    ND_texcoord_vector2(SceneClass const &sceneClass, std::string const &name);
    void update() override;

private:
    static void sample(const Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);

    ispc::ND_texcoord_vector2 mIspc;

RDL2_DSO_CLASS_END(ND_texcoord_vector2)

ND_texcoord_vector2::ND_texcoord_vector2(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    mSampleFunc = ND_texcoord_vector2::sample;
    mSampleFuncv = (SampleFuncv) ispc::ND_texcoord_vector2_getSampleFunc();
}

void
ND_texcoord_vector2::update()
{
    mIspc.disableMode = (get(disableAttr) || (getenv("DISABLE_MATERIALX_STUBS") != nullptr));
    
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
    
    moonray::shading::TypedAttributeKey<Vec2f> stAttributeKey(stName);
    moonray::shading::TypedAttributeKey<Vec2f> uvAttributeKey(uvName);
    
    mIspc.mStAttributeIndex = stAttributeKey;
    mIspc.mUvAttributeIndex = uvAttributeKey;
    
    mOptionalAttributes.push_back(stAttributeKey);
    mOptionalAttributes.push_back(uvAttributeKey);
}

void
ND_texcoord_vector2::sample(const Map* self, moonray::shading::TLState *tls,
                            const moonray::shading::State& state, Color* sample)
{
    const ND_texcoord_vector2* me = static_cast<const ND_texcoord_vector2*>(self);

    Vec2f outValue;
    if (me->mIspc.disableMode) {
        outValue = Vec2f(0.0f, 0.0f);
    } else {
        int stKey = me->mIspc.mStAttributeIndex;
        int uvKey = me->mIspc.mUvAttributeIndex;

        // Check for "st" attribute first
        if (state.isProvided(stKey)) {
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Vec2f>(stKey));
        }
        // If "st" not available, check for "uv" attribute
        else if (state.isProvided(uvKey)) {
            outValue = state.getAttribute(moonray::shading::TypedAttributeKey<Vec2f>(uvKey));
        }
        // Fallback to 2D texture coordinates
        else {
            outValue = state.getSt();
        }
    }
    *sample = Color(outValue.x, -outValue.y, 0.0f);
}

