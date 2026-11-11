// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Renderer.h"
#include <pxr/imaging/hd/renderDelegate.h>

namespace hdMoonray {

class RenderDelegate;


// Render settings are stored generically by the RenderDelegate. The job of this class is
// (a) to know which render settings moonray supports (getDescriptors)
// (b) to take the current values stored by RenderDelegate and apply
//     them to moonray specific scene data (apply). Generally this means modifying the
//     SceneVariables object.
class RenderSettings
{
public:
    RenderSettings(RenderDelegate& delegate) : mDelegate(delegate) {}

    void addDescriptors(pxr::HdRenderSettingDescriptorList& descriptorList) const;

    // if stored settings have changed since the last apply,
    // apply them to the scene. SceneContext must exist, and may be acquired
    void apply();

    // get a render setting
    pxr::VtValue getRenderSetting(const pxr::TfToken& key) const;
    template<typename T> T get(const pxr::TfToken& key) const {
        // Houdini sometimes changes the type (e.g. long in place of int or bool)
        // so we have to cast to the right type.
        //
        // VtValue::Get<T>() on an empty or non-convertible value emits
        // "Coding Error: Attempted to get value of type 'T' from empty VtValue"
        // and then throws. Hydra never guarantees that every setting a host can
        // send is present with the expected type: a Hydra client that only sets
        // a subset of the descriptors (for example this fork's Blender add-on,
        // which sends "executionMode" as a plain string but leaves the Arras
        // transport settings at their defaults) leaves the rest empty here.
        // Fall back to a value-initialised result instead of throwing out of
        // render setup, which previously surfaced as a failed Arras session.
        pxr::VtValue v = getRenderSetting(key);
        if (v.IsEmpty()) {
            return T();
        }
        pxr::VtValue converted = v.Cast<T>();
        return converted.IsEmpty() ? T() : converted.UncheckedGet<T>();
    }

    std::string getExecutionMode() const;
    void setDeepIdAttributeName();

private:

    RenderDelegate& mDelegate;
};

}
