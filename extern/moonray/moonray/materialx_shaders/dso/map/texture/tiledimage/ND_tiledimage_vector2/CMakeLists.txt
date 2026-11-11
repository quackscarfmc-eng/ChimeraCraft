# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

moonray_ispc_dso(ND_tiledimage_vector2
    DEPENDENCIES
        MaterialxShaders::moonray_mtlx_map
        Moonray::common_mcrt_macros
        Moonray::rendering_shading
        Moonray::shading_ispc
        Moonray::texturing_sampler)
add_dependencies(all_shader_dsos ND_tiledimage_vector2)
