# SPDX-FileCopyrightText: 2026 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# -----------------------------------------------------------------------------
# OpenMoonRay runtime bundling (WITH_MOONRAY)
#
# `extern/moonray` is a standalone OpenMoonRay project: it calls
# `cmake_minimum_required()` / `project(openmoonray)` itself, defines its own
# `CMAKE_INSTALL_PREFIX` and uses OpenMoonRay-only CMake macros. It CANNOT be
# added as a subdirectory of Blender's build. MoonRay is therefore built once,
# out of tree, against the *same* USD / Python ABI as Blender, and this module
# installs that prebuilt tree into the layout expected by
# `build_files/package/appimage/AppRun`:
#
#   usr/lib/moonray/installs/openmoonray/
#       plugin/pxr/    (USD plugInfo.json files + delegate libraries)
#       rdl2dso/       (RDL2 DSOs)
#       sessions/      (Arras session definitions)
#       shader_json/   (shader metadata)
#       lib/ lib64/ bin/
#
# Reproducible build (exact USD flags depend on the USD build Blender uses):
#
#   cmake -S extern/moonray -B ../build_openmoonray \
#         -DCMAKE_INSTALL_PREFIX=../build_openmoonray/install \
#         -DUSD_ROOT=<blender-usd-install-root> \
#         -DPYTHON_EXECUTABLE=<same-python-as-blender>
#   cmake --build ../build_openmoonray --target install
#
#   cmake -B ../build_linux -DWITH_MOONRAY=ON -DWITH_USD=ON -DWITH_HYDRA=ON \
#         -DMOONRAY_ROOT=../build_openmoonray/install
#   cmake --build ../build_linux --target install
#   make package_appimage          # see GNUmakefile
#
# `WITH_MOONRAY=OFF` (the default) adds nothing to the build, so a stock
# `make update && make ninja` remains reproducible on all platforms.
# -----------------------------------------------------------------------------

if(NOT WITH_MOONRAY)
  return()
endif()

if(NOT WITH_USD OR NOT WITH_HYDRA)
  message(FATAL_ERROR
    "WITH_MOONRAY requires WITH_USD=ON and WITH_HYDRA=ON "
    "(the delegate is discovered through USD's plugin registry)."
  )
endif()

if(NOT MOONRAY_ROOT OR "${MOONRAY_ROOT}" STREQUAL "")
  message(FATAL_ERROR
    "WITH_MOONRAY=ON requires -DMOONRAY_ROOT=<path-to-built-openmoonray-install>. "
    "See build_files/cmake/moonray.cmake for the out-of-tree build steps."
  )
endif()

if(NOT IS_ABSOLUTE "${MOONRAY_ROOT}")
  get_filename_component(MOONRAY_ROOT "${MOONRAY_ROOT}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
endif()

if(NOT IS_DIRECTORY "${MOONRAY_ROOT}")
  message(FATAL_ERROR "MOONRAY_ROOT is not a directory: \"${MOONRAY_ROOT}\"")
endif()

# The four directories AppRun hard-requires. Validate them at configure time so
# a broken/partial MoonRay tree fails the build instead of producing an
# AppImage that exits at startup with "MoonRay runtime is missing".
set(_moonray_required_dirs plugin/pxr rdl2dso sessions shader_json)
foreach(_dir IN LISTS _moonray_required_dirs)
  if(NOT IS_DIRECTORY "${MOONRAY_ROOT}/${_dir}")
    message(FATAL_ERROR
      "MOONRAY_ROOT is missing required directory \"${_dir}\": "
      "\"${MOONRAY_ROOT}/${_dir}\". "
      "Build and install OpenMoonRay out of tree first."
    )
  endif()
endforeach()
unset(_moonray_required_dirs)

# Staging directory for the AppImage payload. Keep it separate from the normal
# Blender install so `WITH_MOONRAY` never changes a regular install's layout.
set(MOONRAY_APPIMAGE_STAGE "${CMAKE_BINARY_DIR}/package/appimage"
    CACHE PATH "Staging directory for the MoonRay-enabled AppImage payload")

set(_moonray_openmoonray "${MOONRAY_APPIMAGE_STAGE}/usr/lib/moonray/installs/openmoonray")
set(_moonray_deps "${MOONRAY_APPIMAGE_STAGE}/usr/lib/moonray/installs")

# Runtime components. `install(DIRECTORY ...)` copies directory *contents*.
foreach(_dir IN ITEMS plugin rdl2dso sessions shader_json lib lib64 bin)
  if(IS_DIRECTORY "${MOONRAY_ROOT}/${_dir}")
    install(DIRECTORY "${MOONRAY_ROOT}/${_dir}/"
            DESTINATION "${_moonray_openmoonray}/${_dir}"
            COMPONENT moonray_runtime
            USE_SOURCE_PERMISSIONS)
  endif()
endforeach()

# Some OpenMoonRay installs place shared libraries at the top level of the
# installs/ directory (AppRun adds ${DEPS_ROOT}/lib and lib64 to
# LD_LIBRARY_PATH). Copy those too when present.
foreach(_dir IN ITEMS lib lib64)
  if(IS_DIRECTORY "${MOONRAY_ROOT}/../${_dir}")
    install(DIRECTORY "${MOONRAY_ROOT}/../${_dir}/"
            DESTINATION "${_moonray_deps}/${_dir}"
            COMPONENT moonray_runtime
            USE_SOURCE_PERMISSIONS)
  endif()
endforeach()

# Generate a usable hd_moonray plugInfo.json from the checked-in template.
# Only `plugInfo.json.in` plus two `{"Includes": ["*/"]}` files are committed
# in this repository; no in-repo rule previously substituted @LIBRARY_PATH@.
set(_moonray_plugin_template
    "${CMAKE_SOURCE_DIR}/extern/moonray/moonray/hydra/hdMoonray/plugin/hd_moonray/plugInfo.json.in")
if(EXISTS "${_moonray_plugin_template}")
  set(LIBRARY_PATH "../../hd_moonray${CMAKE_SHARED_LIBRARY_SUFFIX}")
  configure_file(
    "${_moonray_plugin_template}"
    "${CMAKE_BINARY_DIR}/moonray/hd_moonray/plugInfo.json"
    @ONLY)
  install(FILES "${CMAKE_BINARY_DIR}/moonray/hd_moonray/plugInfo.json"
          DESTINATION "${_moonray_openmoonray}/plugin/pxr/hd_moonray"
          COMPONENT moonray_runtime)
  unset(LIBRARY_PATH)
endif()

# The AppImage launcher itself. AppRun is executable inside the AppDir.
install(FILES "${CMAKE_SOURCE_DIR}/build_files/package/appimage/AppRun"
        DESTINATION "${MOONRAY_APPIMAGE_STAGE}"
        COMPONENT moonray_runtime
        PERMISSIONS
          OWNER_READ OWNER_WRITE OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)

message(STATUS "WITH_MOONRAY: bundling OpenMoonRay runtime from \"${MOONRAY_ROOT}\"")
message(STATUS "WITH_MOONRAY: AppImage stage \"${MOONRAY_APPIMAGE_STAGE}\"")

unset(_moonray_plugin_template)
unset(_moonray_openmoonray)
unset(_moonray_deps)
