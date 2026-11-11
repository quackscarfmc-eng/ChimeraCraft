# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

set(python_install_dir python/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/${PACKAGE_NAME})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.py DESTINATION ${python_install_dir})
