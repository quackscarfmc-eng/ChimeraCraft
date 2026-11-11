# Copyright 2023-2025 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

# find QT if using moonray_gui
find_package(Qt5
    COMPONENTS
        Core
        Gui
        OpenGL)

if(Qt5_FOUND)
    add_subdirectory(moonray_gui)
endif()

add_subdirectory(moonray_gui_v2)
