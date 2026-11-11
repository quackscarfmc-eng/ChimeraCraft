<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

ChimeraCraft
=======

ChimeraCraft is based on Blender.

Project Pages
-------------

- [Main Website](https://www.blender.org)
- [Reference Manual](https://docs.blender.org/manual/en/latest/index.html)
- [User Community](https://www.blender.org/community/)
- [Code Review & Bug Tracker](https://projects.blender.org)
- [Developer Forum](https://devtalk.blender.org)
- [Developer Documentation](https://developer.blender.org/docs/)

What's new (November 2026)
--------------------------

- **Rebased onto Blender 5.3.0.** The fork's Blender 5.2.2 LTS base was
  replaced with the upstream 5.3.0 release tree. The vendored MoonRay /
  ppf-contact-solver trees were carried verbatim and the 23-file
  Blender-side integration set was replayed with a three-way merge — zero
  conflicts. See `REBASE_5.3.0_REPORT.md` for the full accounting.
- **Repository history squashed at the 5.3.0 base.** Upstream ancestry below
  the 5.3.0 release tree is intentionally not carried; the branch now starts
  from a single root commit of the upstream tree followed by the fork's
  integration commit. This keeps the standalone repository self-contained.
- **MoonRay sampler table excluded from the repository.**
  `pmj02_best_candidate_4096.bin` (128 MiB) exceeds GitHub's 100 MB file
  limit — and GitHub rejects it even via Git LFS — so it is intentionally
  **not tracked**. It is a generated/production data table consumed by
  MoonRay's PMJ02 sampler; restore it for a MoonRay build from OpenMoonRay's
  source tree or regenerate it, then stage it locally. It is listed in
  `.gitignore`. All other binaries (assets, release datafiles, icons, tests)
  are stored as regular Git blobs.
- **MoonRay M1 SIGSEGV fixed in source.** The `mcrt` worker crash in
  `Distribution.cc` (uninitialised `inputPixelBuffer`) is patched in this
  tree; a MoonRay rebuild is required to pick it up in new binaries.
- **ZOZO/PPF contact solver stays reverted.** The unfinished "Step 1.2
  Incomplete" integration was removed from the build;
  `extern/ppf-contact-solver/` remains as an inert Apache-2.0 reference.

Development
-----------

### Prerequisites

Use a full Git checkout and install the platform's C/C++ toolchain, CMake,
Python 3, and Git. Ninja is recommended. The build uses platform-specific
precompiled libraries; run the update command below to fetch them before
building. See the [official build handbook](https://developer.blender.org/docs/handbook/building_blender/)
for detailed toolchain requirements and troubleshooting.

Run commands from the repository root.

### Linux

Install a supported C/C++ compiler and the system development packages required
by Blender, plus CMake, Ninja, and Python 3. Then run:

```sh
make update
make ninja
```

The default x86-64 build is placed in `../build_linux/bin/`. Other architectures
need matching precompiled libraries or locally built dependencies.

### macOS

Install Xcode and its command-line tools, CMake, Ninja, and Python 3. Then run:

```sh
make update
make ninja
```

The application bundle is placed in `../build_darwin/bin/`. This checkout
provides precompiled libraries for Apple Silicon (arm64); Intel Macs need
compatible dependencies built separately.

### Windows

Use a Visual Studio installation with the C++ desktop development workload,
along with Git, CMake, and Python 3. From a Visual Studio Developer Command
Prompt, run:

```bat
make.bat update
make.bat
```

The default build uses Visual Studio/MSBuild and writes to `..\build_windows\`.
To use Ninja instead, install Ninja and run `make.bat ninja`.

### Build notes and limitations

- USD and Hydra are enabled by default. The `hydra_moonray` add-on is bundled
  and enabled automatically, but rendering requires a MoonRay Hydra delegate
  built against Blender's USD 26.03 with a matching Python/ABI.
- A stock `make update && make ninja` does **not** build or bundle MoonRay:
  `extern/moonray` is a standalone OpenMoonRay project and cannot be nested in
  Blender's CMake build. To produce the MoonRay-enabled AppImage reproducibly,
  build OpenMoonRay out of tree and configure Blender with `WITH_MOONRAY=ON`
  and `MOONRAY_ROOT=<install-tree>`, then run `make package_appimage`. See
  `build_files/cmake/moonray.cmake` and
  `scripts/addons_core/hydra_moonray/README.md`. This fork documents MoonRay
  builds for Linux and macOS, but does not claim a native Windows MoonRay
  build. Blender itself can still be built on Windows.
- The AppImage produced by `make package_appimage` bundles the CPU-only
  MoonRay runtime (plugin, `rdl2dso`, sessions and shader JSON) into
  `usr/lib/moonray/installs/openmoonray`. It links against the host glibc and
  therefore needs a glibc at least as new as the one it was built against;
  the AppImage format does not bundle glibc.
- The `SIGSEGV` inside MoonRay's distributed `mcrt` worker (see
  `issue-report.html`, M1) has been **fixed** in this tree. The crash fired
  during the `renderPrep` stage in `ImageDistribution::init` (file
  `extern/moonray/moonray/moonray/lib/rendering/pbr/core/Distribution.cc`).
  The root cause was an uninitialised `float* inputPixelBuffer` (declared
  without `= nullptr`) that was freed unconditionally after the mip-level
  loop. When a 1×1 base mip level hit the workaround `break` before the
  `mipLevel == 0` allocation, the uninitialised pointer was passed to
  `alignedFreeArray`, causing the SIGSEGV inside glibc's free/malloc path.
  The fix (1) initialises `inputPixelBuffer` to `nullptr`, (2) guards the
  cleanup free with a null check, and (3) restricts the 1×1 mip workaround
  to sub-levels only (`mipLevel > 0`) so the base mip is always processed.
  The crash was previously described as "not fixable from this repository's
  source" — the latent bug was always in this tree's copy of
  `Distribution.cc` and is now patched. A MoonRay rebuild is still required
  to produce new binaries; the precompiled `.so` in the AppImage must be
  rebuilt to pick up this fix.
  The Python/Hydra-side `VtValue` defects found while isolating the crash
  *were* also fixed in this tree — see section D of `fix-report.html`.
- The Blender release data files consumed by the install step are **present
  and verified** in this tree: `startup.blend`, `preview.blend`,
  `preview_grease_pencil.blend` and `splash.png` under `release/datafiles/`,
  plus 64 supporting assets (`assets/`, `studiolights/`,
  `colormanagement/icc/`). All 68 were checked byte-for-byte (SHA-256)
  against the official `blender-5.2.2-linux-x64` release. CMake validates
  this at configure time: it fails on a truncated Git-LFS pointer file
  (<1 KB `startup.blend`) and warns if the preview blends or `splash.png`
  are missing. Note that `splash_2x.png` is deliberately **not** required —
  the 5.2 release does not ship it and nothing in the source tree reads it;
  only `splash.png` is embedded (`data_to_c_simple` in
  `source/blender/editors/datafiles/CMakeLists.txt`) and displayed by
  `wm_splash_screen.cc`.
- `.gitignore` still lists `/release/datafiles/assets/` (an upstream rule,
  kept as-is) and the release binaries are Git-LFS objects upstream, so these
  files are present in the working tree but not tracked in this repository.
  `.gitattributes` no longer carries the upstream LFS rules — all binaries in
  this repository are ordinary Git blobs (the `pmj02` sampler table is the one
  exception, tracked separately/excluded per *What's new* above). If you
  need the release datafiles versioned, force-add them
  (`git add -f release/datafiles/assets release/datafiles/*.blend
  release/datafiles/splash.png release/datafiles/studiolights`) rather than
  relaxing the ignore rule.
- Generated build trees (`build_moonray/`, `out/`) and the investigation
  transcript (`codex-session-MoonRay.md`) are ignored by `.gitignore` and are
  **not tracked**: the index holds no entry for any of them, so no
  `git rm --cached` cleanup is outstanding.

### ZOZO / PPF contact solver — reverted

The unfinished in-process ZOZO contact solver integration ("Step 1.2
Incomplete") was **reverted** rather than shipped as dead, OFF-by-default
code. It consisted of an OFF-by-default `WITH_PPF_CONTACT_SOLVER` option, a
Cargo `staticlib` build, one `ED_OT_ppf_solver_status` operator that only
called `begin()`/`end()`, a Physics panel, and a C ABI whose `initialize()`
accepted opaque pointers to Rust-owned `DataSet`/`ParamSet` records with no
constructors — so Blender could never build a scene to solve. The GPU
backends were deliberately disabled and the `cuda-driver` arm is known to
fail at `AbiDevice::open`. A separate `blender_addon/` extension under
`extern/` was never installed and had no call path to the Rust ABI.
Finishing it would require a stable C ABI with constructors, real
`initialize`/`step`/`fetch` operators, RNA/DNA and a frame/depsgraph hook,
and an install rule for the Python extension — none of which can be verified
without a working Rust/Blender toolchain.

Removed from the build: the `WITH_PPF_CONTACT_SOLVER` option and its Cargo
validation, the `source/blender/editors/physics/CMakeLists.txt` custom command
and link block, the operator registration/declaration, the
`properties_physics_ppf` startup module, and the broken `CONFIGURE_DEPENDS`
glob for a non-existent root `build.rs`. The retired C++ and Python files are
left as inert, comment-only placeholders (delete them with `git rm` when the
tree is next cleaned up).

`extern/ppf-contact-solver/` remains as vendored Apache-2.0 upstream source
for reference only: it is not added via `add_subdirectory`, not built, and not
installed. Its `src/lib.rs` no longer textually includes `main.rs` and its
Cargo library target no longer produces a `staticlib`. To drop the whole tree:
`git rm -r extern/ppf-contact-solver`. A future integration must start from a
stable C ABI, not revive the `include!("main.rs")` staticlib shortcut.

### Core Hydra add-ons

`hydra_storm` and `hydra_moonray` in `scripts/addons_core/` are legacy
`bl_info` core add-ons (like `hydra_storm` itself), not extensions, so they do
not need a `blender_manifest.toml`. They are enabled automatically at startup
by `scripts/modules/addon_utils.py` when Blender is built with Hydra support,
which is what makes `HYDRA_MOONRAY` appear under `--factory-startup`.

License
-------

Blender as a whole is licensed under the GNU General Public License,
**Version 2 or later** (GPL-2.0-or-later), consistent with `COPYING`,
`doc/license/GPL-license.txt` and the SPDX headers in `source/`. The vendored
`extern/moonray` (OpenMoonRay) and `extern/ppf-contact-solver` (ZOZO) trees
are Apache-2.0; see the SPDX headers in those trees. Individual files may
have a different but compatible license.

See [blender.org/about/license](https://www.blender.org/about/license) for details.
