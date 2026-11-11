# SPDX-FileCopyrightText: 2026 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

import bpy


class RenderSettingOverride(bpy.types.PropertyGroup):
    key: bpy.props.StringProperty(
        name="Setting",
        description="MoonRay Hydra setting name, or moonray:sceneVariable:<attribute>",
    )
    value_type: bpy.props.EnumProperty(
        name="Type",
        items=(
            ("STRING", "String", "Pass a string value"),
            ("BOOLEAN", "Boolean", "Pass a boolean value"),
            ("INTEGER", "Integer", "Pass an integer value"),
            ("FLOAT", "Float", "Pass a floating-point value"),
            ("VECTOR2", "Vector 2", "Pass a two-component vector"),
            ("VECTOR3", "Vector 3", "Pass a three-component vector"),
            ("VECTOR4", "Vector 4", "Pass a four-component vector"),
        ),
        # Custom keys are setting *names* (for example
        # `moonray:sceneVariable:pixel_samples`), so a string is the correct
        # default.  Previously this defaulted to FLOAT, which caused users to
        # send numeric values for keys that hdMoonray expects as strings.
        default="STRING",
    )
    value_string: bpy.props.StringProperty(name="Value")
    value_bool: bpy.props.BoolProperty(name="Value")
    value_int: bpy.props.IntProperty(name="Value")
    value_float: bpy.props.FloatProperty(name="Value")
    value_vector2: bpy.props.FloatVectorProperty(name="Value", size=2)
    value_vector3: bpy.props.FloatVectorProperty(name="Value", size=3)
    value_vector4: bpy.props.FloatVectorProperty(name="Value", size=4)


class MoonRayRenderSettings(bpy.types.PropertyGroup):
    debug: bpy.props.BoolProperty(
        name="Debug Messages", description="Enable MoonRay debug logging", default=False
    )
    info: bpy.props.BoolProperty(
        name="Info Messages", description="Enable MoonRay informational logging", default=False
    )
    rdl_output: bpy.props.StringProperty(
        name="RDL Output",
        description="Write the translated scene to this RDL file",
        subtype="FILE_PATH",
    )
    disable_lighting: bpy.props.BoolProperty(
        name="Disable Lighting", description="Disable scene lighting", default=False
    )
    double_sided: bpy.props.BoolProperty(
        name="Double Sided", description="Render geometry as double-sided", default=False
    )
    decode_normals: bpy.props.BoolProperty(
        name="Decode Normals", description="Decode incoming normal data", default=False
    )
    enable_motion_blur: bpy.props.BoolProperty(
        name="Motion Blur", description="Enable motion blur", default=True
    )
    prune_willow: bpy.props.BoolProperty(name="Prune Willow Procedurals", default=False)
    prune_fur_deform: bpy.props.BoolProperty(name="Prune Fur Deform", default=False)
    prune_volume: bpy.props.BoolProperty(name="Prune Volumes", default=False)
    prune_wrap_deform: bpy.props.BoolProperty(name="Prune Wrap Deform", default=False)
    prune_curve_deform: bpy.props.BoolProperty(name="Prune Curve Deform", default=False)
    force_polygon: bpy.props.BoolProperty(name="Force Polygons", default=False)
    execution_mode: bpy.props.EnumProperty(
        name="Execution Mode",
        description="MoonRay execution mode",
        items=(
            ("auto", "Auto", "Let MoonRay select the execution mode"),
            ("vectorized", "Vectorized", "Use vectorized CPU execution"),
            ("scalar", "Scalar", "Use scalar CPU execution"),
            ("xpu", "XPU", "Use hybrid CPU/GPU execution"),
        ),
        default="auto",
    )
    use_remote_hosts: bpy.props.BoolProperty(name="Use Remote Hosts", default=False)
    remote_hosts: bpy.props.IntProperty(name="Remote Hosts", default=5, min=0)
    local_reserved_cores: bpy.props.IntProperty(name="Local Reserved Cores", default=1, min=0)
    log_level: bpy.props.IntProperty(name="Remote Log Level", default=1, min=1, max=5)
    max_fps: bpy.props.FloatProperty(name="Max FPS", default=12.0, min=0.0)
    max_connect_retries: bpy.props.IntProperty(name="Max Connect Retries", default=2, min=0)
    enable_denoise: bpy.props.BoolProperty(name="Enable Denoising", default=False)
    enable_oidn: bpy.props.BoolProperty(name="Enable OIDN", default=False)
    denoise_albedo_guiding: bpy.props.BoolProperty(name="Denoise Albedo Guiding", default=False)
    denoise_normal_guiding: bpy.props.BoolProperty(name="Denoise Normal Guiding", default=False)
    overrides: bpy.props.CollectionProperty(type=RenderSettingOverride)
    active_override_index: bpy.props.IntProperty(default=0)


class SceneProperties(bpy.types.PropertyGroup):
    final: bpy.props.PointerProperty(type=MoonRayRenderSettings)
    viewport: bpy.props.PointerProperty(type=MoonRayRenderSettings)

    @classmethod
    def register(cls):
        bpy.types.Scene.hydra_moonray = bpy.props.PointerProperty(
            name="MoonRay",
            description="MoonRay renderer settings",
            type=cls,
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Scene.hydra_moonray


register, unregister = bpy.utils.register_classes_factory(
    (RenderSettingOverride, MoonRayRenderSettings, SceneProperties)
)
