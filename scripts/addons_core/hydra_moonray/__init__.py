# SPDX-FileCopyrightText: 2026 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

bl_info = {
    "name": "Hydra MoonRay render engine",
    "author": "Blender Foundation, MoonRay contributors",
    "version": (1, 0, 0),
    "blender": (5, 2, 0),
    "description": "Render with the OpenMoonRay Hydra render delegate",
    "category": "Render",
}

from . import engine, properties, ui


def register():
    properties.register()
    engine.register()
    ui.register()


def unregister():
    ui.unregister()
    engine.unregister()
    properties.unregister()
