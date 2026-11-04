# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

# Find MKL (Math Kernel Library)
#
find_path(MKL_INCLUDE_DIR
  NAMES mkl.h
  HINTS $ENV{MKL_ROOT}/include /usr/local/include)

find_library(MKL_core_LIBRARY
  NAMES mkl_core
  HINTS $ENV{MKL_ROOT}/lib /usr/local/lib)
find_library(MKL_sequential_LIBRARY
  NAMES mkl_sequential
  HINTS $ENV{MKL_ROOT}/lib /usr/local/lib)
find_library(MKL_intel_lp64_LIBRARY
  NAMES mkl_intel_lp64
  HINTS $ENV{MKL_ROOT}/lib /usr/local/lib)
find_library(MKL_avx_LIBRARY
  NAMES mkl_avx
  HINTS $ENV{MKL_ROOT}/lib /usr/local/lib)
find_library(MKL_def_LIBRARY
  NAMES mkl_def
  HINTS $ENV{MKL_ROOT}/lib /usr/local/lib)
find_library(MKL_iomp5_LIBRARY
  NAMES iomp5
  HINTS $ENV{MKL_ROOT}/lib /usr/local/lib)

set(MKL_LIBRARIES
      MKL_core_LIBRARY
      MKL_sequential_LIBRARY
      MKL_intel_lp64_LIBRARY
      MKL_avx_LIBRARY
      MKL_def_LIBRARY
      MKL_iomp5_LIBRARY
)
mark_as_advanced(MKL_INCLUDE_DIR MKL_LIBRARIES
      MKL_core_LIBRARY MKL_sequential_LIBRARY
      MKL_intel_lp64_LIBRARY MKL_avx_LIBRARY
      MKL_def_LIBRARY MKL_iomp5_LIBRARY)

message(STATUS "Looking for MKL components: ${MKL_LIBRARIES}")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MKL
  REQUIRED_VARS
      MKL_INCLUDE_DIR
      MKL_core_LIBRARY
      MKL_sequential_LIBRARY
      MKL_intel_lp64_LIBRARY
      MKL_avx_LIBRARY
      MKL_def_LIBRARY
      MKL_iomp5_LIBRARY

  # This ensures that if any of the REQUIRED_VARS are not found, the entire MKL
  # package is considered NOT FOUND.  With the move to oneMKL there does not
  # appear to be any notion of COMPONENTS, so this means that we won't find
  # MKL at all if any of the above are missing (-NOTFOUND).
  # We should revisit this to see how we can better support oneAPI MKL
  HANDLE_COMPONENTS
)

if (MKL_FOUND AND NOT TARGET MKL::MKL)
    add_library(MKL::core UNKNOWN IMPORTED)
    set_target_properties(MKL::core PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${MKL_core_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
    add_library(MKL::sequential UNKNOWN IMPORTED)
    set_target_properties(MKL::sequential PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${MKL_sequential_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
    add_library(MKL::intel_lp64 UNKNOWN IMPORTED)
    set_target_properties(MKL::intel_lp64 PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${MKL_intel_lp64_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
    add_library(MKL::avx UNKNOWN IMPORTED)
    set_target_properties(MKL::avx PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${MKL_avx_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
    add_library(MKL::def UNKNOWN IMPORTED)
    set_target_properties(MKL::def PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${MKL_def_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
    add_library(MKL::iomp5 UNKNOWN IMPORTED)
    set_target_properties(MKL::iomp5 PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${MKL_iomp5_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MKL_INCLUDE_DIR}")
    add_library(MKL::MKL INTERFACE IMPORTED)
    set_property(TARGET MKL::MKL PROPERTY
        INTERFACE_LINK_LIBRARIES
          MKL::core
          MKL::sequential
          MKL::intel_lp64
          MKL::avx
          MKL::def
          MKL::iomp5)
      if(DEFINED ENV{MKL_ROOT})
        message(STATUS "Found MKL at $ENV{MKL_ROOT}")
      endif()
endif()


