# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

include(RatsTest)

find_package(Python REQUIRED COMPONENTS Interpreter)

# Add a new PaTS test. [Python-based (render) Acceptance Test]
# -------------------------------------------------------------
#
# Pats is done as a wrapper to add_rats_test(). It takes all of the same
# names arguments as add_rats_test() but adds SCRIPT, which is the path to a
# python script that generates a .usda file for the test. Most arguments are
# passed through unchanged to add_rats_test() with the exception of
#    SCRIPT - which is consumed by add_patse_test() and not passed to add_rats_test()
#    INPUTS - which is automatically set to the .rdla file generated from the .usda file
#
# add_pats_test() also passes DEPENDS_RAW to add_rats_test() which is set to the ctest
# test that converts the .usda file to .rdla. DEPENDS_RAW is used rather than DEPENDS just
# takes a basename and assumes the test is actually the usual bundle of rats test
#
# Each call to this function will
#    1) create a ctest test which generates a .usda file from a python script 
#    2) create a ctest test which converts the .usda file to an .rdla file using hd_usd2rdl
#    3 call add_rats_test() to create the usual update, render, and diff tests for the .rdla file.
#
# A typical PaTS test will comprise 11 or more individual CTests, which can later be run in stages; see below.
#
# ---------------------
#
# TEST NAMES:
# Each test will be named according to the following convention:
#   generate-usd_<basename>
#   convert-usd_<basename>
#   <stage>_<exec_mode>_<basename>[-output]
#
#   * the <stage> token will be update|render|diff|header
#   * the <exec_mode> token will be one of sca|vec|xpu
#   * the [_output] token appears on diff & header stages and will be the name of the image, eg. _scene.exr
#
# ---------------------
#
# LABELS:
# See https://cmake.org/cmake/help/latest/prop_test/LABELS.html
# Each test will have its LABELS property set according to the CTest's stage, with the following convention:
# 'update' labeled CTests will:
#       * Render each test's RDLA scene file multiple times to produce candidate canonical images into a temp directory
#       * Perform a large number of image comparisons between the resulting candidates for each test (using the openimageio idiff tool), gathering and analyzing statistics.
#       * Choose a set of ideal images for each test to serve as canonicals for future runs of the test suite.
#       * This set of canonicals are copied to a subdirectory of the directory specified by the ${RATS_CANONICAL_DIR} cache variable,
#         as well as a file (diff.json) containing tolerances for future comparisons.
#
# 'render' labeled CTests will:
#       * Render the scene to produce images into the build directory under the <execution_mode>/ dir.
#
# 'diff' labeled CTests will:
#       * Execute the 'idiff' command an output image comparing it with previously rendered canonical image of the same name.
#       * Or... compare the header of an output image with previously rendered canonical image of the same name.
#
# ---------------------
function(add_pats_test)

    # The following KEYWORD arguments are supported:
    set(options
        DIFF_HEADERS        # Adds an extra CTest with the 'diff' label to compare the canonical
                            # and result image headers.

        DISABLED            # Disables this test for all execution modes
        DISABLED_SCALAR     # Disables this test for scalar execution mode
        DISABLED_VECTOR     # Disables this test for vector execution mode
        DISABLED_XPU        # Disables this test for xpu execution mode

        NO_SCALAR           # | Execution modes to skip for this test. For example, path guiding is
        NO_VECTOR           # | currently only supported in scalar mode, so tests using path guiding may want
        NO_XPU              # | to pass: NO_VECTOR NO_XPU.

        NO_IMAGE_DIFF       # Do not generate update/diff/header stages for this test.
    )

    set(oneValueArgs
        NAME_SUFFIX         # Suffix to be appended to test base name. This allows adding multiple tests from the same CMakeLists.txt
        OUTPUT              # Name of output image file, will be added to render args as -out <OUTPUT>
        RENDERER            # moonray|hd_render (defaults to moonray)
		SCRIPT              # Path to the usd generation python script (required)
                            # This is the one argument that is not passed through to add_rats_test()
    )

    set(multiValueArgs
        CANONICALS          # List of output files the test produces.
                            # If empty, no canonical/diff/header stages are created for this test.
                            # example: CANONICALS scene.exr aovs.exr more_aovs.exr

        DEPENDS             # List of tests that should be run before this test (for the update and
                            # render stages) when running ctest with multiple jobs (eg. -j N). For example,
                            # for a test that uses checkpoint/resume rendering the resume test should run _after_
                            # the checkpoint test, and should therefore specify the checkpoint test's basename
                            # in its DEPENDS list.
                            # (Note that CTests are always run in the order they are added when -j is omitted,
                            # but specifying an explicit dependency here allows such tests to run in the correct
                            # order when multiple jobs are used via the -J option to the ctest command).

        INPUTS              # Additional rdl files to be passed to the render (probably rarely used)
                            # example: INPUTS scene.rdla scene.rdlb

        DELTAS              # Ordered list of optional delta files for the test.
                            # example: DELTAS deltas.rdla

        ENVIRONMENT         # List of definitions to be available as env vars during test runtime, for example:
                            # ENVIRONMENT TEST_ASSETS_DIR=/some/path ANOTHER_VAR="another value"

        RENDER_ARGS         # List of renderer args to set/override.
        RENDER_ARGS_SCALAR  # | List of renderer args to set/override per execution mode.
        RENDER_ARGS_VECTOR  # | Example: RENDER_ARGS_XPU -scene_var pixel_samples 1 -texture_cache_size 8192
        RENDER_ARGS_XPU     # |
    )

    # parse and validate arguments
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(DEFINED ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Keywords missing values: ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(DEFINED ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unrecognized arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT DEFINED ARG_SCRIPT)
        message(FATAL_ERROR "add_pats_test() requires a SCRIPT argument")
    endif()

    #
    # configure some paths
    #
    # the following line will give us the relative path from the project source
    # dir to the current source dir, which we can use to generate a unique test name for this test
    file(RELATIVE_PATH test_rel_path ${PROJECT_SOURCE_DIR}/tests/ ${CMAKE_CURRENT_SOURCE_DIR})
    set(test_basename ${test_rel_path})
    if(DEFINED ARG_NAME_SUFFIX)
        set(test_basename ${test_rel_path}${ARG_NAME_SUFFIX})
    endif()
    set(SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SCRIPT}")

    # Name the rdla file to be the name of the test with an .rdla extension.
	cmake_path(GET SCRIPT_PATH STEM SCRIPT_STEM)
	set(USDA_FILENAME "${SCRIPT_STEM}.usda")
	set(RDLA_FILENAME "${SCRIPT_STEM}.rdla")

    # add CTests
    set(generate_usd_test_name "generate-usd-${test_basename}")
    add_test(NAME ${generate_usd_test_name}
             COMMAND ${Python_EXECUTABLE} ${SCRIPT_PATH} -o ${USDA_FILENAME}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${generate_usd_test_name} PROPERTIES
             LABELS "rats;render;update")
    set(convert_usd_test_name "convert-usd-${test_basename}")
    add_test(NAME ${convert_usd_test_name}
             COMMAND hd_usd2rdl -in ${USDA_FILENAME} -out ${RDLA_FILENAME}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${convert_usd_test_name} PROPERTIES
             LABELS "rats;render;update")

    # make the conversion test depend on the generation test
    set_tests_properties(${convert_usd_test_name} PROPERTIES DEPENDS ${generate_usd_test_name})

    file(RELATIVE_PATH test_rel_path ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	set(ARG_INPUTS ${test_rel_path}/${RDLA_FILENAME})
    set(ARG_DEPENDS_RAW ${convert_usd_test_name})

    # ctest named arguments don't handle passing of empty values well,
    # so we will build a list of arguments to pass to add_rats_test()
    # and only add arguments that have values
    set(ARG_LIST "")
	list(APPEND DIFF_HEADERS ${ARG_DIFF_HEADERS})
	list(APPEND DISABLED ${ARG_DISABLED})
	list(APPEND DISABLED_SCALAR ${ARG_DISABLED_SCALAR})
	list(APPEND DISABLED_VECTOR ${ARG_DISABLED_VECTOR})
	list(APPEND DISABLED_XPU ${ARG_DISABLED_XPU})
	list(APPEND NO_SCALAR ${ARG_NO_SCALAR})
	list(APPEND NO_VECTOR ${ARG_NO_VECTOR})
	list(APPEND NO_XPU ${ARG_NO_XPU})
	list(APPEND NO_IMAGE_DIFF ${ARG_NO_IMAGE_DIFF})
    if (ARG_NAME_SUFFIX)
        list(APPEND ARG_LIST NAME_SUFFIX ${ARG_NAME_SUFFIX})
    endif()
    if (ARG_OUTPUT)
        list(APPEND ARG_LIST OUTPUT ${ARG_OUTPUT})
    endif()
    if (ARG_RENDERER)
        list(APPEND ARG_LIST RENDERER ${ARG_RENDERER})
    endif()
    if (ARG_CANONICALS)
        list(APPEND ARG_LIST CANONICALS ${ARG_CANONICALS})
    endif()
    if (ARG_DEPENDS_RAW)
        list(APPEND ARG_LIST DEPENDS_RAW ${ARG_DEPENDS_RAW})
    endif()
    if (ARG_INPUTS)
        list(APPEND ARG_LIST INPUTS ${ARG_INPUTS})
    endif()
    if (ARG_DELTAS)
        list(APPEND ARG_LIST DELTAS ${ARG_DELTAS})
    endif()
    if (ARG_ENVIRONMENT)
        list(APPEND ARG_LIST ENVIRONMENT ${ARG_ENVIRONMENT})
    endif()
    if (ARG_RENDER_ARGS)
        list(APPEND ARG_LIST RENDER_ARGS ${ARG_RENDER_ARGS})
    endif()
    if (ARG_RENDER_ARGS_SCALAR)
        list(APPEND ARG_LIST RENDER_ARGS_SCALAR ${ARG_RENDER_ARGS_SCALAR})
    endif()
    if (ARG_RENDER_ARGS_VECTOR)
        list(APPEND ARG_LIST RENDER_ARGS_VECTOR ${ARG_RENDER_ARGS_VECTOR})
    endif()
    if (ARG_RENDER_ARGS_XPU)
        list(APPEND ARG_LIST RENDER_ARGS_XPU ${ARG_RENDER_ARGS_XPU})
    endif()

    add_rats_test(${ARG_LIST})

endfunction()
