// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "GuiTypes.h"

#include <mcrt_denoise/denoiser/Denoiser.h>
#include <moonray/rendering/rndr/rndr.h>

namespace moonray_gui_v2 {

using namespace moonray::denoiser;

class DenoiserManager {
public:
    DenoiserManager() {};
    ~DenoiserManager() {};

    // Select which AOV buffers to use for denoising based on available AOVs
    void selectBuffers(const moonray::rndr::RenderContext* renderContext);

    // Toggle denoising on/off
    void toggleDenoising();

    // Cycle through denoising modes
    void toggleDenoisingMode();

    // Denoise the given render buffer
    void denoiseFrame(scene_rdl2::fb_util::RenderBuffer *renderBuffer,
                      const moonray::rndr::RenderContext* renderContext);

    // Gets whether denoising is enabled
    bool getDenoisingEnabled() const { return mDenoise; }

    // Gets which denoise mode is being used
    DenoiserMode getDenoiserMode() const { return mDenoiserMode; }

    // Gets which AOV buffers are being used for denoising
    DenoisingBufferMode getDenoisingBufferMode() const { return mDenoisingBufferMode; }

private:
    // Check if denoiser config has changed
    bool hasChanged(const DenoiserMode& mode, const int w, const int h, 
                    const bool useAlbedo, const bool useNormals) const;

    bool mDenoise {false};                                         // Whether denoising is enabled
    DenoisingBufferMode mDenoisingBufferMode {DN_BUFFERS_BEAUTY};  // Current denoising buffer mode
    std::vector<DenoisingBufferMode> mValidDenoisingBufferModes {};// Valid denoise buffer modes based on available AOVs

#if PLATFORM_APPLE
    DenoiserMode mDenoiserMode {METAL};                            // Current denoiser mode
#else
    DenoiserMode mDenoiserMode {OPTIX};
#endif

    std::unique_ptr<Denoiser> mDenoiser;                           // Denoiser instance
};

} // end namespace moonray_gui_v2