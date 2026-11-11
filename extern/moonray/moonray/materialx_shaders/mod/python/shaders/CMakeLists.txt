# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

# if building at the same time as scene_rdl2 we need to get access to the
# scene_rdl2 python module before it is installed so point to it in the
# build directory. Otherwie it should already be in the environment
if (EXISTS "${CMAKE_BINARY_DIR}/moonray/scene_rdl2" AND IS_DIRECTORY "${CMAKE_BINARY_DIR}/moonray/scene_rdl2")
    set(PYTHONPATH_WITH_SCENE_RDL2 ${CMAKE_BINARY_DIR}/moonray/scene_rdl2/mod/python/py_scene_rdl2:$ENV{PYTHONPATH})
elseif(EXISTS "${CMAKE_BINARY_DIR}/openmoonray/moonray/scene_rdl2" AND IS_DIRECTORY "${CMAKE_BINARY_DIR}/openmoonray/moonray/scene_rdl2")
    set(PYTHONPATH_WITH_SCENE_RDL2 ${CMAKE_BINARY_DIR}/openmoonray/moonray/scene_rdl2/mod/python/py_scene_rdl2:$ENV{PYTHONPATH})
else()
    set(PYTHONPATH_WITH_SCENE_RDL2 $ENV{PYTHONPATH})
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders.py
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${PYTHONPATH_WITH_SCENE_RDL2} RDL2_DSO_PATH=${CMAKE_BINARY_DIR}/rdl2dso:$ENV{RDL2_DSO_PATH} ${GENERATE_PYTHON_CREATE_FUNCTIONS}  -o ${CMAKE_CURRENT_BINARY_DIR}/shaders.py
    COMMENT "Generating shaders.py"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../generate_python_create_functions/generate_python_create_functions"
)

add_custom_target(
	shaders ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shaders.py
)

add_dependencies(shaders all_shader_dsos)

if (EXISTS "${CMAKE_BINARY_DIR}/moonray/scene_rdl2" AND IS_DIRECTORY "${CMAKE_BINARY_DIR}/moonray/scene_rdl2")
	add_dependencies(shaders scene_rdl2)
	add_dependencies(shaders DwaEmissiveMaterial CheckerboardMap TestInputsMap TestBoolOutputMap TestColorOutputMap TestDisplacement2Map TestInputsMap TestIntOutputMap TestMat3fOutputMap TestMat4fOutputMap TestNormalMap2Map TestRgbaOutputMap TestVec4fOutputMap TestInputsDisplacement TestInputsNormalMap )
elseif(EXISTS "${CMAKE_BINARY_DIR}/openmoonray/moonray/scene_rdl2" AND IS_DIRECTORY "${CMAKE_BINARY_DIR}/openmoonray/moonray/scene_rdl2")
	add_dependencies(shaders scene_rdl2)
	add_dependencies(shaders DwaEmissiveMaterial CheckerboardMap TestInputsMap TestBoolOutputMap TestColorOutputMap TestDisplacement2Map TestInputsMap TestIntOutputMap TestMat3fOutputMap TestMat4fOutputMap TestNormalMap2Map TestRgbaOutputMap TestVec4fOutputMap TestInputsDisplacement TestInputsNormalMap )
endif()

set(python_install_dir
		python/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/${PACKAGE_NAME})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/shaders.py DESTINATION ${python_install_dir})
