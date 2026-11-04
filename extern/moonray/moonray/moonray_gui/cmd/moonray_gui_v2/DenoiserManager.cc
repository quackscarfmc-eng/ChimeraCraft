// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "DenoiserManager.h"

#include <moonray/rendering/rndr/RenderOutputDriver.h>

namespace moonray_gui_v2 {

using namespace moonray::denoiser;

/// TODO: We need to do better concurrency management across multiple threads. The RenderGui and Viewport are 
/// running on different threads, and independently access the DenoiserManager, though the Viewport is only 
/// changing the denoising parameters.

void
DenoiserManager::selectBuffers(const moonray::rndr::RenderContext* renderContext)
{
    // populate valid denoising modes based on AOVs in rdla file
    if (mValidDenoisingBufferModes.empty()) {
        mValidDenoisingBufferModes.push_back(DN_BUFFERS_BEAUTY);
        mDenoisingBufferMode = DN_BUFFERS_BEAUTY;
    
        const moonray::rndr::RenderOutputDriver *rod = renderContext->getRenderOutputDriver();
        const bool albedoValid = rod->getDenoiserAlbedoInput() >= 0;
        const bool normalsValid = rod->getDenoiserNormalInput() >= 0;
    
        if (albedoValid) {
            mValidDenoisingBufferModes.push_back(DN_BUFFERS_BEAUTY_ALBEDO);
        }
        if (albedoValid && normalsValid) {
            mValidDenoisingBufferModes.push_back(DN_BUFFERS_BEAUTY_ALBEDO_NORMALS);
        }
    }
    
    // find the next denoising mode    
    do {
        mDenoisingBufferMode = (DenoisingBufferMode)((int)(mDenoisingBufferMode + 1) % NUM_DENOISING_BUFFER_MODES);
    } while (std::find(mValidDenoisingBufferModes.begin(),
                       mValidDenoisingBufferModes.end(),
                       mDenoisingBufferMode ) == mValidDenoisingBufferModes.end());
    
    MNRY_ASSERT(mDenoisingBufferMode == DN_BUFFERS_BEAUTY || 
                mDenoisingBufferMode == DN_BUFFERS_BEAUTY_ALBEDO ||
                mDenoisingBufferMode == DN_BUFFERS_BEAUTY_ALBEDO_NORMALS);
}

void
DenoiserManager::toggleDenoising()
{
    mDenoise = !mDenoise;
}

void
DenoiserManager::toggleDenoisingMode()
{
    if (mDenoiserMode == OPTIX) {
        mDenoiserMode = OPEN_IMAGE_DENOISE;
    } else if (mDenoiserMode == METAL) {
        mDenoiserMode = OPEN_IMAGE_DENOISE_CPU;
    } else if (mDenoiserMode == OPEN_IMAGE_DENOISE) {
        mDenoiserMode = OPEN_IMAGE_DENOISE_CPU;
    } else if (mDenoiserMode == OPEN_IMAGE_DENOISE_CPU) {
        mDenoiserMode = OPEN_IMAGE_DENOISE_CUDA;
    } else {
        mDenoiserMode = OPTIX;
    }
}

bool
DenoiserManager::hasChanged(const DenoiserMode& mode, const int w, const int h, 
                            const bool useAlbedo, const bool useNormals) const
{
    return mode != mDenoiser->mode() || 
           w != mDenoiser->imageWidth() || 
           h != mDenoiser->imageHeight() ||
           useAlbedo != mDenoiser->useAlbedo() ||
           useNormals != mDenoiser->useNormals();
}

void
DenoiserManager::denoiseFrame(scene_rdl2::fb_util::RenderBuffer *renderBuffer,
                              const moonray::rndr::RenderContext* renderContext)
{
    // Early out if denoising is disabled
    if (!mDenoise) { return; }

    const unsigned w = renderBuffer->getWidth();
    const unsigned h = renderBuffer->getHeight();

    const moonray::rndr::RenderOutputDriver *rod = renderContext->getRenderOutputDriver();
    const int albedoIndx = rod->getDenoiserAlbedoInput();
    const int normalIndx = rod->getDenoiserNormalInput();

    const bool useAlbedo = (albedoIndx >= 0 && mDenoisingBufferMode != DN_BUFFERS_BEAUTY);
    const bool useNormals = (albedoIndx >= 0 && normalIndx >= 0) && 
                            mDenoisingBufferMode == DN_BUFFERS_BEAUTY_ALBEDO_NORMALS;

    // Recreate denoiser if not yet created or config has changed
    if (mDenoiser == nullptr || hasChanged(mDenoiserMode, w, h, useAlbedo, useNormals)) {
        std::string errorMsg;
        mDenoiser = std::make_unique<Denoiser>(mDenoiserMode, w, h, useAlbedo, useNormals, &errorMsg);
        if (!errorMsg.empty()) {
            std::cout << "Error creating denoiser: " << errorMsg << std::endl;
            mDenoiser.release();
        }
    }

    if (mDenoiser) {
        scene_rdl2::fb_util::RenderBuffer albedoBuffer;
        scene_rdl2::fb_util::RenderBuffer normalBuffer;

        if (useAlbedo) {
            renderContext->snapshotAovBuffer(&albedoBuffer, rod->getAovBuffer(albedoIndx), true, false);
        }
        if (useNormals) {
            renderContext->snapshotAovBuffer(&normalBuffer, rod->getAovBuffer(normalIndx), true, false);
        }

        const scene_rdl2::fb_util::RenderColor *inputBeautyPixels = renderBuffer->getData();
        const scene_rdl2::fb_util::RenderColor *inputAlbedoPixels = useAlbedo ? albedoBuffer.getData() : nullptr;
        const scene_rdl2::fb_util::RenderColor *inputNormalPixels = useNormals ? normalBuffer.getData() : nullptr;
        scene_rdl2::fb_util::RenderColor *denoisedPixels = renderBuffer->getData();
        std::string errorMsg;

        mDenoiser->denoise(reinterpret_cast<const float*>(inputBeautyPixels),
                           reinterpret_cast<const float*>(inputAlbedoPixels),
                           reinterpret_cast<const float*>(inputNormalPixels),
                           reinterpret_cast<float*>(denoisedPixels),
                           &errorMsg);

        if (!errorMsg.empty()) {
            std::cout << "Error denoising: " << errorMsg << std::endl;
        }
    }
}

} // end namespace moonray_gui_v2