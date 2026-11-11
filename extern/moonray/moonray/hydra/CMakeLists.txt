# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

find_package(OpenGL REQUIRED)

if(NOT pxr_FOUND)
    find_package(pxr REQUIRED)
endif()

# Monolithic OpenUSD exports its component targets as interfaces. Link the
# shared library as well so standalone Hydra tools resolve USD symbols.
if(TARGET usd_m)
    link_libraries(usd_m)
endif()

add_subdirectory(hdMoonray)
add_subdirectory(moonray_sdr_plugins)
