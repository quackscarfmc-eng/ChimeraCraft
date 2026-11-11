# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

function(${PROJECT_NAME}_link_options target)
    target_link_options(${target}
        PRIVATE
            -Wl,--enable-new-dtags              # Use RUNPATH instead of RPATH
    )
endfunction()
