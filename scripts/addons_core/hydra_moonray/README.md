# Hydra MoonRay render engine

This add-on registers Blender's `MoonRay` render engine (`HYDRA_MOONRAY`) using
the OpenMoonRay `HdMoonrayRendererPlugin` Hydra delegate. It is a bundled core
add-on and is enabled automatically at startup when Blender is built with
Hydra, so **MoonRay** is available in Render Properties even under
`--factory-startup`.

> A second, older `release/scripts/addons_core/hdmoonray` add-on registered a
> no-op `MOONRAY` engine with the same "MoonRay" label. It has been retired and
> registers nothing; use `HYDRA_MOONRAY`.

## Runtime requirements

Blender must be built with USD and Hydra support (`WITH_USD=ON`,
`WITH_HYDRA=ON`). MoonRay and its Hydra delegate are runtime dependencies, not
linked into Blender. Install a MoonRay build compatible with Blender's USD
version and make the delegate and its shared libraries discoverable before
starting Blender:

- Set `PXR_PLUGINPATH_NAME` to the directory containing the MoonRay USD
  `plugInfo.json` files. This is the canonical USD plugin-discovery variable
  for all USD versions (OpenMoonRay's own `scripts/setup.sh` uses it); USD
  26.03 additionally honours the spelling `USD_PLUG_IN_PATH`, so exporting
  both is the safest option. `PXR_PLUGIN_PATH` is not a USD variable — do
  not use it.
- Configure the platform's shared-library search path for MoonRay's
  libraries (the bundled delegate additionally needs `libopenvdb.so.9.1`,
  OpenImageIO 2.4, OpenSubdiv 3.5, boost 1.75, JSONCPP 25, log4cplus 2.0,
  lua 5.4, libpython3.9 and a USD v22.11 `libusd_ms.so`), and set
  `RDL2_DSO_PATH`, `ARRAS_SESSION_PATH` and `MOONRAY_CLASS_PATH` as the
  bundled `AppRun` does.

The bundled MoonRay documentation describes Linux and macOS builds. A working
MoonRay Hydra delegate is required at runtime to render; this fork does not
claim a native Windows MoonRay build.

## Reproducible bundling

`extern/moonray` is a standalone OpenMoonRay project and cannot be nested into
Blender's CMake build. Build it out of tree against the **same USD / Python
ABI** as Blender, then let Blender install it into the AppImage layout:

```sh
cmake -S extern/moonray -B ../build_openmoonray \
      -DCMAKE_INSTALL_PREFIX=../build_openmoonray/install \
      -DUSD_ROOT=<blender-usd-install-root>
cmake --build ../build_openmoonray --target install

cmake -B ../build_linux -DWITH_MOONRAY=ON -DWITH_USD=ON -DWITH_HYDRA=ON \
      -DMOONRAY_ROOT=../build_openmoonray/install
cmake --build ../build_linux --target install
make package_appimage
```

`WITH_MOONRAY` is `OFF` by default, so a stock `make update && make ninja`
does not depend on MoonRay. See `build_files/cmake/moonray.cmake` for the exact
layout `AppRun` expects.

> Rendering (`MCRT` / Arras) still requires a MoonRay build with a symbolised,
> ABI-matched `mcrt` worker. The recorded `SIGSEGV` in the distributed worker
> cannot be fixed from this repository's source; see the issue report.

## Render settings

The Render Properties panels expose the MoonRay and Arras settings published by
`hdMoonray`, including execution mode, distributed rendering, denoising, motion
blur, geometry options, logging, and RDL output. Material previews are
disabled (`bl_use_preview = False`) because MoonRay is a CPU path tracer, and
MaterialX export is disabled (`bl_use_materialx = False`) because hdMoonray
translates UsdPreviewSurface and `BUILD_MATERIALX_SHADERS` is off.

**Custom Render Settings** accepts additional Hydra setting names and typed
scalar/vector values. Keys default to the `STRING` type, which is what
`moonray:sceneVariable:<attribute_name>` overrides need:

```text
moonray:sceneVariable:pixel_samples
```

Custom keys may not shadow the built-in setting names (that would send a
wrong-typed value into the delegate), and duplicate keys are ignored rather
than aborting the render. The **Render** and **Viewport** panels each edit
their own settings group. Scene-variable names and valid values depend on the
MoonRay version and scene schema installed with the delegate.
