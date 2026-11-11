# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project


set(component moonray_mtlx_map)

set(installIncludeDir ${PACKAGE_NAME}/map)
set(exportGroup ${PROJECT_NAME}Targets)

add_library(${component} SHARED "")
add_library(${PROJECT_NAME}::${component} ALIAS ${component})

# ----------------------------------------
# compile some ispc sources to object files
set(objLib ${component}_objlib)

add_library(${objLib} OBJECT)

target_sources(${objLib}
    PRIVATE
        ispc/MtlxHextile.ispc
        ispc/MtlxVariantTypes.ispc
)

file(RELATIVE_PATH relBinDir ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(
    ${objLib}
    PROPERTIES
        ISPC_HEADER_SUFFIX _ispc_stubs.h
        ISPC_HEADER_DIRECTORY /${relBinDir}
        ISPC_INSTRUCTION_SETS avx2-i32x8
        # ISPC_INSTRUCTION_SETS ${GLOBAL_ISPC_INSTRUCTION_SETS}
        LINKER_LANGUAGE CXX
)


target_link_libraries(${objLib}
    PRIVATE
        Moonray::common_file_resource
        Moonray::common_mcrt_util
        Moonray::shading_eval_ispc
        Moonray::shading_ispc
        Moonray::texturing_sampler
        SceneRdl2::scene_rdl2
)

# Set standard compile/link options
MaterialxShaders_ispc_compile_options(${objLib})

get_target_property(objLibDeps ${objLib} DEPENDENCY)
if(NOT objLibDeps STREQUAL "")
    add_dependencies(${objLibDeps}
        Moonray::common_file_resource
        Moonray::common_mcrt_util
        Moonray::shading_eval_ispc
        Moonray::shading_ispc
        Moonray::texturing_sampler
        SceneRdl2::scene_rdl2
    )
endif()

# ---

get_target_property(ISPC_TARGET_OBJECTS ${objLib} TARGET_OBJECTS)
target_sources(${component}
    PRIVATE
        MtlxHextile.cc
        MtlxVariantTypes.cc
        # pull in our ispc object files
        ${ISPC_TARGET_OBJECTS}
)

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
        MtlxHextile.h
        MtlxVariantTypes.h
        # ${CMAKE_CURRENT_BINARY_DIR}/MtlxHextile_ispc_stubs.h
        ${CMAKE_CURRENT_BINARY_DIR}/MtlxVariantTypes_ispc_stubs.h
)

set_property(TARGET ${component}
    PROPERTY PRIVATE_HEADER
        ispc/MtlxHextile.isph
        ispc/MtlxVariantTypes.isph
)


target_include_directories(${component}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${component}
    PUBLIC
        Moonray::common_file_resource
        Moonray::common_mcrt_util
        Moonray::shading_eval_ispc
        Moonray::shading_ispc
        Moonray::texturing_sampler
        SceneRdl2::scene_rdl2
)

add_dependencies(${component} ${objLib})

# If at Dreamworks add a SConscript stub file so others can use this library.
SConscript_Stub(${component})

# Set standard compile/link options
MaterialxShaders_cxx_compile_definitions(${component})
MaterialxShaders_cxx_compile_features(${component})
MaterialxShaders_cxx_compile_options(${component})
MaterialxShaders_link_options(${component})

# -------------------------------------
# Install the target and the export set
# -------------------------------------
include(GNUInstallDirs)

# install the target
install(TARGETS ${component}
    COMPONENT ${component}
    EXPORT ${exportGroup}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        NAMELINK_SKIP
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${installIncludeDir}
)

# # install the export set
# install(
#     EXPORT ${exportGroup}
#     NAMESPACE ${PROJECT_NAME}::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
# )
