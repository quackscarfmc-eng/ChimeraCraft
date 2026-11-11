# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

moonray_ispc_dso(ND_frame_float
    DEPENDENCIES
        Moonray::common_mcrt_macros
        Moonray::rendering_shading
        Moonray::shading_ispc)
add_dependencies(all_shader_dsos ND_frame_float)
