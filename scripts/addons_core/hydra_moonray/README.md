# Hydra MoonRay render engine

This add-on registers Blender's `MoonRay` render engine using the OpenMoonRay
`HdMoonrayRendererPlugin` Hydra delegate. Enable the add-on in Preferences, then
select **MoonRay** in Render Properties.

## Runtime requirements

Blender must be built with USD and Hydra support (`WITH_USD=ON`,
`WITH_HYDRA=ON`). MoonRay and its Hydra delegate are runtime dependencies, not
linked into Blender. Install a MoonRay build compatible with Blender's USD
version and make the delegate and its shared libraries discoverable before
starting Blender. Set `PXR_PLUGINPATH_NAME` to the directory containing the
MoonRay USD plugInfo file, and configure the platform's shared-library search
path for MoonRay's libraries.

The bundled MoonRay documentation describes Linux and macOS builds. This
optional add-on does not add MoonRay dependencies to Blender's build, so Blender
continues to build on Windows, macOS, and Linux. A working MoonRay Hydra
delegate is required at runtime to render; this fork does not claim a native
Windows MoonRay build.

## Render settings

The Render Properties panel exposes the MoonRay and Arras settings published by
`hdMoonray`, including execution mode, distributed rendering, denoising, motion
blur, geometry options, logging, and RDL output. **Custom Render Settings**
accepts additional Hydra setting names and typed scalar/vector values. To
override a MoonRay scene variable, use a key of the form
`moonray:sceneVariable:<attribute_name>`. For example:

```text
moonray:sceneVariable:pixel_samples
```

Scene-variable names and valid values depend on the MoonRay version and scene
schema installed with the delegate.
