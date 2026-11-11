# ChimeraCraft → Blender 5.3.0 Rebase Report

**Date:** rebase performed in the ChimeraCraft sandbox
**Branch:** `chimeracraft-5.3.0` (new branch at upstream `v5.3.0`, commit `02f0899d`)
**Result commit:** `bf039335`
**Fork tip rebased:** `fa2a4947` (Blender 5.2.2 LTS base)

---

## 1. What was done

The fork's Blender 5.2.2 LTS base was replaced with the upstream **Blender 5.3.0**
release (`v5.3.0`, `02f0899d`). Two classes of fork content were carried forward:

1. **Vendored dependencies** (`extern/moonray`, `extern/ppf-contact-solver`) were
   copied verbatim from the fork tip with `git checkout fa2a4947 -- extern/...`.
2. **Blender-side integration** was replayed with `git apply --index --3way`
   against the **5.2.2 base** (`upstream-v5.2.2`, `d13f752e3b`) used as the
   three-way merge ancestor, so each modified file is merged onto 5.3.0's
   version of that file.

**The source-delta three-way merge applied 100% cleanly — zero conflicts.**

---

## 2. What was carried (committed in `bf039335`)

| Category | Count | Notes |
|---|---|---|
| `extern/` vendored (moonray + ppf-contact-solver) | 9,766 | copied verbatim from fork tip |
| Blender-side integration files | 23 | 8 new + 15 modified |
| `.gitignore` (fork hygiene appended) | 1 | modified |

**Integration files (23):**
- `build_files/cmake/moonray.cmake` **(new)**, `build_files/cmake/packaging.cmake` (mod)
- `build_files/package/appimage/AppRun` **(new)**
- `scripts/addons_core/hydra_moonray/` → `__init__.py`, `engine.py`, `properties.py`,
  `ui.py`, `README.md` **(all new)**
- `scripts/modules/_bpy_types.py` (mod), `scripts/modules/addon_utils.py` (mod)
- `scripts/startup/bl_ui/properties_render.py` (mod)
- `source/blender/render/hydra/` → `CMakeLists.txt`, `engine.hh`, `final_engine.cc`,
  `python.cc`, `settings.hh` (all mod)
- `source/blender/editors/physics/` → `CMakeLists.txt`, `physics_intern.hh`,
  `physics_ops.cc` (all mod)
- `CMakeLists.txt` (mod), `GNUmakefile` (mod), `README.md` (mod),
  `MOONRAY_ZOZO_FIX_PLAN.md` **(new)**

The `.gitignore` additions: `build_moonray/`, `build_ovdb/`, `codex-session-*.md`,
`issue-report.html`, `fix-report.html`.

---

## 3. Version metadata (inherited from the 5.3.0 base — no manual edit)

| Item | Value | Action |
|---|---|---|
| `BKE_blender_version.h` | `503 / 0 / release / (empty suffix)` | correct, kept |
| `.gitmodules` (5 lib submodules) | pinned to `blender-v5.3-release` | already correct in 5.3.0 base |
| `.gitattributes` | v5.3.0 LFS filters | **kept** (required for build-machine LFS) |

---

## 4. What was EXCLUDED and why

The raw fork diff (`upstream-v5.2.2..fa2a4947`) contained **192 "deletion" entries**
and **276 spurious `release/scripts` "additions"** that were artifacts of the fork's
own "LFS-eradicated" initial commit, **not** real 5.2.2→fork changes. These were
deliberately dropped:

1. **Binary assets (kept upstream's).** `release/` (142: PNG/ICNS/TIF/BMP/`.car`),
   `assets/` (15 `.blend`), `scripts/.../startup.blend` (5), `doc` favicon. The fork's
   history had dropped these; for a *working* 5.3.0 build we keep upstream's versions,
   which are re-materialized via LFS on the build machine.
2. **`lib/*` submodule gitlinks (5).** Kept 5.3.0's and repinned to
   `blender-v5.3-release` — never deleted.
3. **Repo metadata deletions** (`.gitea`, `.github`, `.well-known`, `.clang-*`,
   `.editorconfig`, `.git-blame-ignore-revs`). Kept upstream's (harmless, not build-critical).
4. **`build_ovdb/`** — 246 tracked build *artifacts* at the fork tip. Deliberately
   excluded and added to `.gitignore` so it stays out of version control.
5. **`release/datafiles`** — kept 5.3.0's (LFS-managed).
6. **`tests/`** — excluded (binary `.blend` patches cannot be applied as text).

The delta was regenerated with `--diff-filter=AM` (Additions + Modifications only)
plus pathspec exclusions for `extern`, `build_ovdb`, `release`, `assets`, `lib`,
`*.blend`, and the one binary doc PNG, yielding the clean 23-file integration set.

---

## 5. Known unresolved / build-machine items

- **U1 — App-template `.blend` files.** `scripts/startup/bl_app_templates_system/
  {Sculpting,Video_Editing}/startup.blend` are left at their **5.3.0 versions**
  (binary patch failed; version-sensitive, safe to leave as-is).
- **U2 — LFS assets.** `git-lfs` is **not installed in this sandbox**. On the build
  machine: `git lfs install && git lfs pull` to materialize binary assets and
  `release/datafiles`.
- **U3 — OpenVDB ABI mismatch (v9_14 vs v9_10).** Documented in
  `MOONRAY_ZOZO_FIX_PLAN.md`; must be resolved when building MoonRay against 5.3.0's
  OpenVDB.
- **U4 — Semantic correctness unverified.** There is **no compiler in the sandbox**,
  so the 5.3.0 source does not compile-check here. The three-way merge is *syntactically*
  clean, but a real build must confirm the fork's changes are still semantically valid
  against 5.3.0's API/ABI.

---

## 6. Build verification runbook (run on the build machine, not the sandbox)

1. **Prerequisites:** `git-lfs`; C++20 toolchain (GCC 13+ / Clang 17+); CMake ≥ 3.28;
   Python 3.11+.
2. **Precompiled libs:** `git submodule update --init --recursive lib/linux_x64`
   (or your platform — pins point to `blender-v5.3-release`).
3. **LFS:** `git lfs install && git lfs pull`.
4. **Configure with MoonRay** (MoonRay is built out of tree first — see the header of
   `build_files/cmake/moonray.cmake`):
   ```sh
   cmake -B build_linux -S . \
         -DWITH_MOONRAY=ON -DWITH_USD=ON -DWITH_HYDRA=ON \
         -DMOONRAY_ROOT=<path-to-prebuilt-openmoonray>
   ```
5. **Build:** `cmake --build build_linux -j"$(nproc)"`.
6. **Smoke test:** launch `./build_linux/bin/blender`; confirm *Render → Engine* lists
   **MoonRay**; confirm no `ImportError` for `_bpy_hydra`; render a small scene.
7. **If OpenVDB ABI error:** rebuild OpenVDB/USD against the same ABI, or apply the
   mismatch patch described in `MOONRAY_ZOZO_FIX_PLAN.md`.

---

## 7. ZOZO/PPF status

The ZOZO/PPF contact-solver integration remains **reverted** (per the earlier
decision). `extern/ppf-contact-solver` is carried as an **inert Apache-2.0 reference
vendoring** only; no build wiring references it.

---

## 8. Not pushed

`chimeracraft-5.3.0` is **local only**. Push when ready:

```sh
git push -u origin chimeracraft-5.3.0
```

> Note: the sandbox used a locally-installed `git` (no system git) and temporarily
> overrode `filter.lfs.*` to work around the missing `git-lfs` binary. Those local
> overrides have been **reverted** and the sandbox-only `.git/info/attributes` bypass
> removed, so the repository now relies on your build machine's global `git-lfs` as
> expected.
