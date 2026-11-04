# SPDX-FileCopyrightText: 2026 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

import bpy


class MoonRayHydraRenderEngine(bpy.types.HydraRenderEngine):
    bl_idname = "HYDRA_MOONRAY"
    bl_label = "MoonRay"
    bl_info = "OpenMoonRay production path tracer"

    bl_use_preview = True
    bl_use_materialx = True
    bl_delegate_id = "HdMoonrayRendererPlugin"

    def get_render_settings(self, engine_type):
        scene_settings = bpy.context.scene.hydra_moonray
        settings = scene_settings.viewport if engine_type == "VIEWPORT" else scene_settings.final

        result = {
            "debug": settings.debug,
            "info": settings.info,
            "rdlOutput": settings.rdl_output,
            "disableLighting": settings.disable_lighting,
            "doubleSided": settings.double_sided,
            "decodeNormals": settings.decode_normals,
            "enableMotionBlur": settings.enable_motion_blur,
            "pruneWillow": settings.prune_willow,
            "pruneFurDeform": settings.prune_fur_deform,
            "pruneVolume": settings.prune_volume,
            "pruneWrapDeform": settings.prune_wrap_deform,
            "pruneCurveDeform": settings.prune_curve_deform,
            "forcePolygon": settings.force_polygon,
            "executionMode": settings.execution_mode,
            "useRemoteHosts": settings.use_remote_hosts,
            "remoteHosts": settings.remote_hosts,
            "localReservedCores": settings.local_reserved_cores,
            "logLevel": settings.log_level,
            "maxFps": settings.max_fps,
            "maxConnectRetries": settings.max_connect_retries,
            "enableDenoise": settings.enable_denoise,
            "enableOIDN": settings.enable_oidn,
            "denoiseAlbedoGuiding": settings.denoise_albedo_guiding,
            "denoiseNormalGuiding": settings.denoise_normal_guiding,
        }

        custom_keys = set()
        for override in settings.overrides:
            if not override.key.strip():
                continue
            key = override.key.strip()
            if key in custom_keys:
                raise ValueError(f"Duplicate MoonRay render setting: {key}")
            custom_keys.add(key)

            if override.value_type == "BOOLEAN":
                value = override.value_bool
            elif override.value_type == "INTEGER":
                value = override.value_int
            elif override.value_type == "FLOAT":
                value = override.value_float
            elif override.value_type == "VECTOR2":
                value = tuple(override.value_vector2)
            elif override.value_type == "VECTOR3":
                value = tuple(override.value_vector3)
            elif override.value_type == "VECTOR4":
                value = tuple(override.value_vector4)
            else:
                value = override.value_string
            result[key] = value

        if engine_type != "VIEWPORT":
            result["aovToken:Combined"] = "color"
            result["aovToken:Depth"] = "depth"

        return result

    def update_render_passes(self, scene, render_layer):
        if render_layer.use_pass_combined:
            self.register_pass(scene, render_layer, "Combined", 4, "RGBA", "COLOR")
        if render_layer.use_pass_z:
            self.register_pass(scene, render_layer, "Depth", 1, "Z", "VALUE")


register, unregister = bpy.utils.register_classes_factory((MoonRayHydraRenderEngine,))
