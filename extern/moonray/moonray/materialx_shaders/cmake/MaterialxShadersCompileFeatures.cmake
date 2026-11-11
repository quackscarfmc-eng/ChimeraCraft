# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

function(${PROJECT_NAME}_cxx_compile_features target)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL Intel)
        target_compile_features(${target}
            PRIVATE
            cxx_std_17
            )
    endif()
endfunction()
