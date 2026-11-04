# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

add_pats_test(
    SCRIPT Test_distance.py
    CANONICALS scene.exr
    RENDER_ARGS -size 600 800
)
