# ChimeraCraft MoonRay Renderer & ZOZO Contact Solver - Issue Analysis & Fix Plan

## Executive Summary

The ChimeraCraft AppImage recognizes MoonRay as a render delegate but fails to render due to **ABI incompatibilities** and **missing plugin components**. The ZOZO/PPF contact solver integration was reverted and is non-functional.

---

## Part 1: MoonRay Renderer Issues

### Issue 1.1: OpenVDB ABI Mismatch (CRITICAL - Blocks All Rendering)

**Error:**
```
Error: .../librendering_shading.so: undefined symbol: _ZN7openvdb4v9_14math8internal4half8_toFloatE
```

**Root Cause:**
- MoonRay was built against **OpenVDB 9.1 with inline namespace `v9_14`** (`openvdb::v9_14::math::internal::half::_toFloat()`)
- The AppImage bundles **OpenVDB 9.1 with inline namespace `v9_10`** (`openvdb::v9_10::math::internal::half::_toFloat()`)
- OpenVDB uses inline namespaces for ABI versioning; `v9_14` ≠ `v9_10` = **ABI incompatibility**

**Evidence:**
```bash
# MoonRay expects (from librendering_shading.so):
_ZN7openvdb4v9_14math8internal4half8_toFloatE  →  openvdb::v9_14::math::internal::half::_toFloat()

# Bundled OpenVDB provides (from deps/lib/libopenvdb.so.9.1.0):
_ZN7openvdb5v9_104math8internal4half8_toFloatE  →  openvdb::v9_10::math::internal::half::_toFloat()
```

**Impact:** All MoonRay computation DSOs fail to load → Arras session creation fails → Rendering impossible.

**Fix Options:**

| Option | Description | Effort | Risk |
|--------|-------------|--------|------|
| **A: Rebuild MoonRay against Blender's OpenVDB 13.0** | Update MoonRay CMake to use Blender's USD/OpenVDB 13.0; may require MoonRay source patches | High | Medium (MoonRay may not support OpenVDB 13 yet) |
| **B: Bundle correct OpenVDB 9.1 (v9_14)** | Obtain/build OpenVDB 9.1 with v9_14 namespace; replace deps/lib version | Medium | Low |
| **C: Use Blender's OpenVDB 13.0 with compat layer** | Build MoonRay with OPENVDB_ABI_VERSION=14 or similar compat define | Medium | High (untested) |

**Recommended:** **Option B** - Source OpenVDB 9.1 with v9_14 namespace (the exact version MoonRay was built against) and bundle it in `deps/lib/`, ensuring it loads before Blender's OpenVDB 13.0.

**Implementation Steps:**
1. Identify exact OpenVDB commit/tag used for MoonRay build (check `extern/moonray` build logs or CI)
2. Build OpenVDB 9.1 with `OPENVDB_ABI_VERSION=14` (or equivalent)
3. Replace `/usr/lib/moonray/installs/deps/lib/libopenvdb.so.9.1*` in AppImage staging
4. Verify symbol `_ZN7openvdb4v9_14math8internal4half8_toFloatE` exists in new library
5. Test MoonRay render

---

### Issue 1.2: Missing SDR Plugin Python Modules (HIGH - Causes Warnings & Potential Failures)

**Error:**
```
Warning: Error loading lib moonrayShaderParser's module 'pxr.MoonrayShaderParser':
ModuleNotFoundError: No module named 'pxr.MoonrayShaderParser'

Warning: Error loading lib moonrayShaderDiscovery's module 'pxr.MoonrayShaderDiscovery':
ModuleNotFoundError: No module named 'pxr.MoonrayShaderDiscovery'
```

**Root Cause:**
- `moonrayShaderParser` and `moonrayShaderDiscovery` are USD SDR (Shader Discovery) plugins
- Each has a `__init__.py` file in source (`extern/moonray/moonray/hydra/moonray_sdr_plugins/*/`)
- CMakeLists.txt installs the C++ `.so` and `plugInfo.json` but **NOT the `__init__.py` files**
- USD's `TfScriptModuleLoader` tries to load them as Python modules under `pxr.*` namespace

**Evidence:**
```python
# Source files exist but not installed:
extern/moonray/moonray/hydra/moonray_sdr_plugins/moonrayShaderParser/__init__.py
extern/moonray/moonray/hydra/moonray_sdr_plugins/moonrayShaderDiscovery/__init__.py

# Installed location (missing __init__.py):
/usr/lib/moonray/installs/openmoonray/plugin/pxr/moonrayShaderParser/plugInfo.json
/usr/lib/moonray/installs/openmoonray/plugin/pxr/moonrayShaderDiscovery/plugInfo.json
```

**Fix:** Modify MoonRay CMakeLists.txt for both plugins to install `__init__.py` alongside `plugInfo.json`.

**Implementation Steps:**
1. Edit `extern/moonray/moonray/hydra/moonray_sdr_plugins/moonrayShaderParser/CMakeLists.txt`
2. Edit `extern/moonray/moonray/hydra/moonray_sdr_plugins/moonrayShaderDiscovery/CMakeLists.txt`
3. Add `install(FILES __init__.py DESTINATION plugin/pxr/moonrayShaderParser)` (and Discovery)
4. Rebuild MoonRay and regenerate AppImage

---

### Issue 1.3: AppRun Library Path Ordering (MEDIUM - Potential Conflict)

**Current LD_LIBRARY_PATH Order:**
```bash
export LD_LIBRARY_PATH="$APPDIR/usr/lib:$APPDIR/usr/bin/lib:$DEPS_ROOT/deps/lib:$DEPS_ROOT/lib:$DEPS_ROOT/lib64:$MOONRAY_ROOT/lib:$MOONRAY_ROOT/lib64"
```

**Problem:** Blender's `usr/bin/lib` (OpenVDB 13.0, OIIO 3.1, OSL 1.15) comes **before** MoonRay's `deps/lib` (OpenVDB 9.1, OIIO 2.4, OSL not bundled). While sonames differ (`.so.13.0` vs `.so.9.1`), this ordering can cause:
- Symbol resolution conflicts for libraries with same soname
- MoonRay plugins loading Blender's incompatible libraries

**Fix:** Reorder to prioritize MoonRay deps:
```bash
export LD_LIBRARY_PATH="$DEPS_ROOT/deps/lib:$DEPS_ROOT/lib:$DEPS_ROOT/lib64:$MOONRAY_ROOT/lib:$MOONRAY_ROOT/lib64:$APPDIR/usr/lib:$APPDIR/usr/bin/lib"
```

**Implementation:** Edit `build_files/package/appimage/AppRun`

---

### Issue 1.4: MoonRay Build Configuration Drift (MEDIUM)

**Observation:** The MoonRay build in `extern/moonray` uses its own CMake configuration independent of Blender's USD/OpenVDB versions. The AppImage build process (`moonray.cmake`) copies a prebuilt MoonRay install tree but doesn't verify ABI compatibility with Blender's bundled libraries.

**Fix:** Add CMake validation in `moonray.cmake` to check:
- OpenVDB version/ABI matches between MoonRay and Blender
- OpenImageIO version compatibility
- OpenSubdiv version compatibility
- Python version compatibility (MoonRay uses 3.9, Blender uses 3.13)

---

## Part 2: ZOZO / PPF Contact Solver Issues

### Issue 2.1: Integration Fully Reverted (NOT A BUG - BY DESIGN)

**Status:** The ZOZO contact solver integration was **intentionally reverted** (see README.md lines 136-166).

**What Was Removed:**
- `WITH_PPF_CONTACT_SOLVER` CMake option
- Cargo `staticlib` build custom command
- `ED_OT_ppf_solver_status` operator (only called `begin()`/`end()`)
- Physics panel UI
- C ABI with opaque pointers to Rust-owned `DataSet`/`ParamSet` (no constructors)
- `blender_addon/` Python extension (never installed, no call path)

**Why It Was Reverted:**
1. **No stable C ABI** - Rust `staticlib` with `include!("main.rs")` is not a stable ABI
2. **No scene construction** - C ABI accepted Rust-owned pointers with no constructors; Blender could never build a scene
3. **GPU backends disabled** - CUDA driver arm fails at `AbiDevice::open`
4. **No frame/depsgraph hook** - No integration with Blender's simulation pipeline
5. **Unverifiable without working Rust/Blender toolchain**

**Current State:** `extern/ppf-contact-solver/` remains as vendored Apache-2.0 upstream source for reference only.

---

### Issue 2.2: Requirements for Future Integration (DOCUMENTATION)

To properly integrate ZOZO contact solver, the following are **minimum requirements**:

#### 2.2.1 Stable C ABI
```c
// Required ABI functions (example):
typedef struct PPF_DataSet PPF_DataSet;
typedef struct PPF_ParamSet PPF_ParamSet;

PPF_DataSet* ppf_dataset_create(void);
void ppf_dataset_destroy(PPF_DataSet*);
int ppf_dataset_add_collider(PPF_DataSet*, const PPF_ColliderDesc*);
int ppf_dataset_add_rod(PPF_DataSet*, const PPF_RodDesc*);

PPF_ParamSet* ppf_params_create(void);
void ppf_params_destroy(PPF_ParamSet*);

int ppf_solver_initialize(PPF_DataSet*, PPF_ParamSet*);
int ppf_solver_step(PPF_DataSet*, float dt);
int ppf_solver_fetch(PPF_DataSet*, PPF_Result*);
```

#### 2.2.2 Blender Integration Points
- **RNA/DNA**: Define `PhysicsSettingsPPF`, `PointCachePPF`, `ObjectPPFSettings`
- **Operators**: `PPF_OT_bake`, `PPF_OT_free`, `PPF_OT_solver_status`
- **Depsgraph Hook**: Frame-change callback to call `ppf_solver_step()`
- **UI Panel**: Physics properties panel for PPF settings

#### 2.2.3 Build System
- Cargo `cdylib` (not `staticlib`) for dynamic loading
- CMake `find_package` for Rust toolchain
- Install rules for `.so` + Python extension

#### 2.2.4 GPU Support
- Fix `cuda-driver` feature in `ppf-cts-solver`
- Implement `AbiDevice::open` for CUDA
- Optional: Metal (macOS), Vulkan (cross-platform)

#### 2.2.5 Estimated Effort
| Component | Effort |
|-----------|--------|
| Stable C ABI + Rust cdylib | 2-3 weeks |
| Blender RNA/DNA + operators | 2-3 weeks |
| Depsgraph integration | 1-2 weeks |
| UI panel | 1 week |
| GPU backend (CUDA) | 2-4 weeks |
| **Total** | **8-15 weeks** |

---

## Part 3: Prioritized Fix Plan

### Phase 1: Critical MoonRay Fixes (Week 1-2)
| Priority | Task | Owner | Dependencies |
|----------|------|-------|--------------|
| P0 | Obtain/build OpenVDB 9.1 with v9_14 namespace | Build Engineer | MoonRay build logs |
| P0 | Replace deps/lib OpenVDB in AppImage staging | Build Engineer | P0 complete |
| P0 | Test MoonRay render (verify Arras session starts) | QA | P0 complete |
| P1 | Install SDR plugin `__init__.py` files | Build Engineer | MoonRay source access |
| P1 | Rebuild MoonRay with fixed CMakeLists.txt | Build Engineer | P1 complete |
| P1 | Regenerate AppImage | Build Engineer | P1 complete |

### Phase 2: MoonRay Stability (Week 2-3)
| Priority | Task | Owner | Dependencies |
|----------|------|-------|--------------|
| P2 | Fix AppRun LD_LIBRARY_PATH ordering | Build Engineer | None |
| P2 | Add CMake ABI validation in moonray.cmake | Build Engineer | None |
| P2 | Test with complex scenes (volumes, curves, instancing) | QA | Phase 1 complete |
| P3 | Document MoonRay build/rebuild process | Docs | Phase 1 complete |

### Phase 3: ZOZO Contact Solver (Future - Requires Decision)
| Priority | Task | Owner | Dependencies |
|----------|------|-------|--------------|
| Future | Decision: Invest in ZOZO integration? | Project Lead | Business case |
| Future | If yes: Design stable C ABI | Rust Engineer | Phase 1-2 complete |
| Future | If yes: Implement Blender RNA/DNA/operators | Blender Engineer | C ABI stable |
| Future | If yes: GPU backend support | Graphics Engineer | C ABI stable |

---

## Part 4: Testing Checklist

### MoonRay Rendering Tests
- [ ] Basic render (cube + light) → Combined + Depth passes
- [ ] Volume rendering (OpenVDB volumes)
- [ ] Curve/hair rendering
- [ ] Instancing (particles, geometry)
- [ ] Motion blur
- [ ] Denoising (OIDN)
- [ ] Arras distributed rendering (multi-machine)
- [ ] MaterialX shader support (if BUILD_MATERIALX_SHADERS=ON)
- [ ] Viewport rendering (Hydra viewport delegate)

### AppImage Tests
- [ ] AppRun starts without "MoonRay runtime missing" error
- [ ] All MoonRay libraries load (ldd on hd_moonray.so)
- [ ] USD plugin discovery works (PXR_PLUGINPATH_NAME)
- [ ] No Python module warnings for MoonrayShaderParser/Discovery
- [ ] Blender factory startup works
- [ ] Render engine selector shows "MoonRay"

---

## Part 5: Files to Modify

### MoonRay Source (extern/moonray/)
```
extern/moonray/moonray/hydra/moonray_sdr_plugins/moonrayShaderParser/CMakeLists.txt
extern/moonray/moonray/hydra/moonray_sdr_plugins/moonrayShaderDiscovery/CMakeLists.txt
```

### Blender Build Config
```
build_files/cmake/moonray.cmake          # Add ABI validation
build_files/package/appimage/AppRun       # Fix LD_LIBRARY_PATH ordering
```

### AppImage Staging (regenerated after MoonRay rebuild)
```
usr/lib/moonray/installs/deps/lib/libopenvdb.so.9.1*  # Replace with v9_14 version
usr/lib/moonray/installs/openmoonray/plugin/pxr/moonrayShaderParser/__init__.py
usr/lib/moonray/installs/openmoonray/plugin/pxr/moonrayShaderDiscovery/__init__.py
```

---

## Part 6: Quick Verification Commands

```bash
# 1. Check OpenVDB symbols in MoonRay lib
nm -D /path/to/librendering_shading.so | grep "openvdb.*half.*toFloat"

# 2. Check OpenVDB symbols in bundled lib
nm -D /path/to/deps/lib/libopenvdb.so.9.1.0 | grep "openvdb.*half.*toFloat"

# 3. Test MoonRay plugin load
LD_LIBRARY_PATH=... ldd /path/to/hd_moonray.so | grep "not found"

# 4. Test render (factory startup)
./AppRun --background --factory-startup --python-expr "
import bpy
bpy.ops.preferences.addon_enable(module='hydra_moonray')
bpy.context.scene.render.engine = 'HYDRA_MOONRAY'
bpy.ops.render.render(write_still=False)
print('SUCCESS')
"

# 5. Check SDR plugin Python modules
python3 -c "import sys; sys.path.insert(0, '/path/to/plugin/pxr'); import MoonrayShaderParser; print('OK')"
```

---

## Conclusion

**MoonRay rendering is blocked by a single critical issue: OpenVDB ABI mismatch (v9_14 vs v9_10).** Once the correct OpenVDB 9.1 build is bundled, rendering should work. The SDR plugin Python modules are a secondary fix to eliminate warnings.

**ZOZO contact solver is not a bug - it was intentionally reverted.** Re-integrating it would be a major new feature requiring 8-15 weeks of engineering effort with a stable Rust/Blender toolchain.

**Recommended immediate action:** Focus on Phase 1 MoonRay fixes to unblock rendering. Defer ZOZO to a separate project with proper scoping.