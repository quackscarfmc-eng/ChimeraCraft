// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "GuiTypes.h"

#include <scene_rdl2/common/fb_util/PixelBufferUtilsGamma8bit.h>

#include <OpenColorIO/OpenColorIO.h>

namespace OCIO = OCIO_NAMESPACE;

namespace moonray_gui_v2 {

class Viewport;

class ColorManager 
{
public:
    ColorManager();
    ~ColorManager();

    void applyCRT(const Viewport* viewport,
                  const int renderOutput, 
                  const scene_rdl2::fb_util::RenderBuffer& renderBuffer, 
                  const scene_rdl2::fb_util::VariablePixelBuffer& renderOutputBuffer,
                  scene_rdl2::fb_util::Rgb888Buffer* displayBuffer, 
                  const scene_rdl2::fb_util::PixelBufferUtilOptions options, 
                  const bool parallel) const;

    void applySnapshotCRT(const Viewport* viewport,
                          float* pixelData,
                          const int width, 
                          const int height,
                          scene_rdl2::fb_util::Rgb888Buffer* displayBuffer) const;
    
    void setupConfig();

private:

    OCIO::ConstConfigRcPtr mConfig;
    bool mConfigIsRaw;

    // read config, define OCIO transforms, initialize processors 
    void configureOcio(const double exposure, 
                        const double gamma, 
                        const DebugMode mode, 
                        OCIO::ConstCPUProcessorRcPtr& cpuProcessor) const;

    // apply color transformations using OCIO
    static void applyCRT_Ocio(const OCIO::ConstCPUProcessorRcPtr& cpuProcessor, 
                                void* srcData, 
                                scene_rdl2::fb_util::Rgb888Buffer* destBuf,
                                const int w, const int h, 
                                const int channels);

    // apply color transformations using previous non-OCIO code
    static void applyCRT_Legacy(const scene_rdl2::fb_util::RenderBuffer& renderBuffer, 
                                const scene_rdl2::fb_util::VariablePixelBuffer& renderOutputBuffer,
                                scene_rdl2::fb_util::Rgb888Buffer* displayBuffer, 
                                const int renderOutput,
                                const double exposure, 
                                const double gamma, 
                                const DebugMode mode,
                                scene_rdl2::fb_util::PixelBufferUtilOptions options, 
                                const bool parallel);
}; 
}

