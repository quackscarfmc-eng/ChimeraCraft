# Custom Blender Engine Registration for MoonRay
import bpy

bl_info = {
    "name": "MoonRay Render Engine",
    "author": "DreamWorks / ChimeraCraft",
    "version": (1, 0, 0),
    "blender": (4, 2, 0),
    "category": "Render",
    "description": "DreamWorks MoonRay MCRT Path Tracing Engine Integration",
}

class MoonRayRenderEngine(bpy.types.RenderEngine):
    bl_idname = "MOONRAY"
    bl_label = "MoonRay"
    bl_use_preview = True

    def render(self, depsgraph):
        pass

def register():
    bpy.utils.register_class(MoonRayRenderEngine)

def unregister():
    bpy.utils.unregister_class(MoonRayRenderEngine)

if __name__ == "__main__":
    register()
