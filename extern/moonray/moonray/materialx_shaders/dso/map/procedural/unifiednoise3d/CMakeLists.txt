# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

moonray_ispc_dso(ND_unifiednoise3d_float
DEPENDENCIES
    Moonray::common_mcrt_macros
    Moonray::common_noise
    Moonray::rendering_shading
    Moonray::shading_ispc
    SceneRdl2::render_util)
add_dependencies(all_shader_dsos ND_unifiednoise3d_float)
