# SPDX-FileCopyrightText: 2026 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

import bpy

from .engine import MoonRayHydraRenderEngine


class RenderPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"
    COMPAT_ENGINES = {MoonRayHydraRenderEngine.bl_idname}

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class MOONRAY_RENDER_PT_main(RenderPanel, bpy.types.Panel):
    bl_label = "MoonRay"

    def draw(self, _context):
        pass


class MOONRAY_RENDER_PT_final(RenderPanel, bpy.types.Panel):
    bl_label = "Render"
    bl_parent_id = "MOONRAY_RENDER_PT_main"

    def draw(self, context):
        settings = context.scene.hydra_moonray.final
        draw_settings(self.layout, settings, "FINAL")


class MOONRAY_RENDER_PT_viewport(RenderPanel, bpy.types.Panel):
    bl_label = "Viewport"
    bl_parent_id = "MOONRAY_RENDER_PT_main"

    def draw(self, context):
        settings = context.scene.hydra_moonray.viewport
        draw_settings(self.layout, settings, "VIEWPORT")


def draw_settings(layout, settings, settings_mode):
    layout.use_property_split = True
    layout.use_property_decorate = False

    layout.prop(settings, "execution_mode")
    layout.prop(settings, "enable_motion_blur")

    distributed = layout.column(heading="Distributed Rendering", align=True)
    distributed.prop(settings, "use_remote_hosts")
    remote = distributed.column(align=True)
    remote.active = settings.use_remote_hosts
    remote.prop(settings, "remote_hosts")
    distributed.prop(settings, "local_reserved_cores")
    distributed.prop(settings, "log_level")
    distributed.prop(settings, "max_connect_retries")
    distributed.prop(settings, "max_fps")

    denoising = layout.column(heading="Denoising", align=True)
    denoising.prop(settings, "enable_denoise")
    oidn = denoising.column(align=True)
    oidn.active = settings.enable_denoise or settings.enable_oidn
    oidn.prop(settings, "enable_oidn")
    oidn.prop(settings, "denoise_albedo_guiding")
    oidn.prop(settings, "denoise_normal_guiding")

    lighting = layout.column(heading="Lighting", align=True)
    lighting.prop(settings, "disable_lighting")
    lighting.prop(settings, "double_sided")
    lighting.prop(settings, "decode_normals")

    geometry = layout.column(heading="Geometry", align=True)
    geometry.prop(settings, "force_polygon")
    geometry.prop(settings, "prune_willow")
    geometry.prop(settings, "prune_fur_deform")
    geometry.prop(settings, "prune_volume")
    geometry.prop(settings, "prune_wrap_deform")
    geometry.prop(settings, "prune_curve_deform")

    advanced = layout.column(heading="Diagnostics", align=True)
    advanced.prop(settings, "debug")
    advanced.prop(settings, "info")
    advanced.prop(settings, "rdl_output")

    layout.separator()
    layout.label(text="Custom Render Settings")
    row = layout.row()
    row.template_list(
        "MOONRAY_UL_render_setting_overrides",
        "",
        settings,
        "overrides",
        settings,
        "active_override_index",
        rows=4,
    )
    buttons = row.column(align=True)
    # The operator must know which settings group the button belongs to;
    # otherwise both panels would edit the final settings (M10).
    add_op = buttons.operator("hydra_moonray.add_render_setting", icon="ADD", text="")
    add_op.settings_mode = settings_mode
    remove_op = buttons.operator("hydra_moonray.remove_render_setting", icon="REMOVE", text="")
    remove_op.settings_mode = settings_mode
    layout.label(
        text="Use moonray:sceneVariable:<name> to override a MoonRay scene variable.",
        icon="INFO",
    )


class MOONRAY_UL_render_setting_overrides(bpy.types.UIList):
    def draw_item(
        self, _context, layout, _data, item, _icon, _active_data, _active_property, _index
    ):
        row = layout.row(align=True)
        row.prop(item, "key", text="")
        row.prop(item, "value_type", text="")
        value_property = {
            "STRING": "value_string",
            "BOOLEAN": "value_bool",
            "INTEGER": "value_int",
            "FLOAT": "value_float",
            "VECTOR2": "value_vector2",
            "VECTOR3": "value_vector3",
            "VECTOR4": "value_vector4",
        }.get(item.value_type, "value_string")
        row.prop(item, value_property, text="")


# Which Scene.hydra_moonray settings group an override operator edits.
# Set by the panel that draws the button so the Viewport panel never edits the
# final settings (and vice versa).
SETTINGS_MODE_ITEMS = (
    ("FINAL", "Final", "Final render settings"),
    ("VIEWPORT", "Viewport", "Viewport render settings"),
)


class HYDRA_MOONRAY_OT_add_render_setting(bpy.types.Operator):
    bl_idname = "hydra_moonray.add_render_setting"
    bl_label = "Add MoonRay Setting"
    bl_options = {"INTERNAL"}

    settings_mode: bpy.props.EnumProperty(
        items=SETTINGS_MODE_ITEMS,
        default="FINAL",
        options={"HIDDEN"},
    )

    def execute(self, context):
        settings = get_active_settings(context, self.settings_mode)
        settings.overrides.add()
        settings.active_override_index = len(settings.overrides) - 1
        return {"FINISHED"}


class HYDRA_MOONRAY_OT_remove_render_setting(bpy.types.Operator):
    bl_idname = "hydra_moonray.remove_render_setting"
    bl_label = "Remove MoonRay Setting"
    bl_options = {"INTERNAL"}

    settings_mode: bpy.props.EnumProperty(
        items=SETTINGS_MODE_ITEMS,
        default="FINAL",
        options={"HIDDEN"},
    )

    @classmethod
    def poll(cls, context):
        return context.engine in RenderPanel.COMPAT_ENGINES

    def execute(self, context):
        settings = get_active_settings(context, self.settings_mode)
        if settings is None or len(settings.overrides) == 0:
            return {"CANCELLED"}
        index = max(0, min(settings.active_override_index, len(settings.overrides) - 1))
        settings.overrides.remove(index)
        # `min(index, len - 1)` becomes -1 when the last override is removed.
        # Clamp to 0 so the UIList's active index stays in range.
        settings.active_override_index = max(0, min(index, len(settings.overrides) - 1))
        return {"FINISHED"}


def get_active_settings(context, settings_mode="FINAL"):
    scene_settings = context.scene.hydra_moonray
    if settings_mode == "VIEWPORT":
        return scene_settings.viewport
    return scene_settings.final


def register():
    bpy.utils.register_class(MOONRAY_UL_render_setting_overrides)
    bpy.utils.register_class(HYDRA_MOONRAY_OT_add_render_setting)
    bpy.utils.register_class(HYDRA_MOONRAY_OT_remove_render_setting)
    for panel in (MOONRAY_RENDER_PT_main, MOONRAY_RENDER_PT_final, MOONRAY_RENDER_PT_viewport):
        bpy.utils.register_class(panel)

    for panel in get_panels():
        panel.COMPAT_ENGINES.add(MoonRayHydraRenderEngine.bl_idname)


def unregister():
    for panel in get_panels():
        panel.COMPAT_ENGINES.discard(MoonRayHydraRenderEngine.bl_idname)

    for panel in (MOONRAY_RENDER_PT_viewport, MOONRAY_RENDER_PT_final, MOONRAY_RENDER_PT_main):
        bpy.utils.unregister_class(panel)
    bpy.utils.unregister_class(HYDRA_MOONRAY_OT_remove_render_setting)
    bpy.utils.unregister_class(HYDRA_MOONRAY_OT_add_render_setting)
    bpy.utils.unregister_class(MOONRAY_UL_render_setting_overrides)


def get_panels():
    exclude_panels = {
        "RENDER_PT_stamp",
        "DATA_PT_light",
        "DATA_PT_spot",
        "NODE_DATA_PT_light",
        "DATA_PT_falloff_curve",
        "RENDER_PT_post_processing",
        "RENDER_PT_simplify",
        "SCENE_PT_audio",
        "RENDER_PT_freestyle",
    }
    include_eevee_panels = {
        "MATERIAL_PT_preview",
        "EEVEE_MATERIAL_PT_context_material",
        "EEVEE_MATERIAL_PT_surface",
        "EEVEE_MATERIAL_PT_volume",
        "EEVEE_MATERIAL_PT_settings",
        "EEVEE_WORLD_PT_surface",
    }

    for panel in bpy.types.Panel.__subclasses__():
        compatible_engines = getattr(panel, "COMPAT_ENGINES", None)
        if compatible_engines is None:
            continue
        if (
            ("BLENDER_RENDER" in compatible_engines and panel.__name__ not in exclude_panels)
            or (
                "BLENDER_EEVEE" in compatible_engines
                and panel.__name__ in include_eevee_panels
            )
        ):
            yield panel
