// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "ArrasRenderer.h"
#include <hydramoonray/NullRenderer.h>
#include <hydramoonray/RenderDelegate.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>

#include <cstdlib>
#include <iostream>
#include <string>

PXR_NAMESPACE_OPEN_SCOPE // this does not work unless inside the pxr namespace

namespace {

// MoonRay's Arras renderer loads its session definitions from
// ARRAS_SESSION_PATH at render time (SessionDefinition::load("hd_single")).
// Advertising the plugin as universally supported when that runtime is absent
// is what previously let it supersede other render delegates and then fail
// with an opaque Arras DefinitionLoadError / MCRT crash. Refuse to claim
// support unless the runtime is actually usable.
bool runtime_available(std::string* reason)
{
    const char* session_path = std::getenv("ARRAS_SESSION_PATH");
    const char* dso_path = std::getenv("RDL2_DSO_PATH");
    if (session_path == nullptr || session_path[0] == '\0' ||
        dso_path == nullptr || dso_path[0] == '\0') {
        if (reason != nullptr) {
            *reason = "MoonRay runtime is unavailable: ARRAS_SESSION_PATH and "
                      "RDL2_DSO_PATH must point at the installed MoonRay "
                      "sessions and rdl2dso directories";
        }
        return false;
    }
    return true;
}

} // namespace

class HdMoonrayRendererPlugin final : public pxr::HdRendererPlugin {
public:
    HdMoonrayRendererPlugin() {}

    pxr::HdRenderDelegate *CreateRenderDelegate() override {
        return new hdMoonray::RenderDelegate(new hdMoonray::ArrasRenderer());
    }

    pxr::HdRenderDelegate *CreateRenderDelegate(pxr::HdRenderSettingsMap const& settings) override {
        auto it = settings.find(pxr::TfToken("disableRender"));
        if (it != settings.end()) {
            if (it->second.Get<bool>()) {
                auto rd = new hdMoonray::RenderDelegate(new hdMoonray::NullRenderer(),settings);
                rd->setDisableRender(true);
                return rd;
            }
        }
        return new hdMoonray::RenderDelegate(new hdMoonray::ArrasRenderer(),settings);
    }

    void DeleteRenderDelegate(pxr::HdRenderDelegate *renderDelegate) override {
        delete renderDelegate;
    }
#if PXR_VERSION >= 2600
    bool IsSupported(pxr::HdRendererCreateArgs const&,
                     std::string* reason) const override {
        return runtime_available(reason);
    }
#elif PXR_VERSION >= 2302
    bool IsSupported(bool gpuEnabled = true) const override {
        return runtime_available(nullptr);
    }
#else
bool IsSupported() const override {
        return runtime_available(nullptr);
    }
#endif

private:
    // uncopyable
    HdMoonrayRendererPlugin(const HdMoonrayRendererPlugin&)             = delete;
    HdMoonrayRendererPlugin &operator =(const HdMoonrayRendererPlugin&) = delete;
};

TF_REGISTRY_FUNCTION(TfType)
{
    pxr::HdRendererPluginRegistry::Define<HdMoonrayRendererPlugin>();
}

PXR_NAMESPACE_CLOSE_SCOPE
