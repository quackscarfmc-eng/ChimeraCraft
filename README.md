<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blender
=======

Blender is the free and open source 3D creation suite.
It supports the entirety of the 3D pipeline—modeling, rigging, animation, simulation, rendering, compositing,
motion tracking and video editing.

![Blender screenshot](https://code.blender.org/wp-content/uploads/2018/12/springrg.jpg "Blender screenshot")

Project Pages
-------------

- [Main Website](https://www.blender.org)
- [Reference Manual](https://docs.blender.org/manual/en/latest/index.html)
- [User Community](https://www.blender.org/community/)

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

- USD and Hydra are enabled by default. To enable the MoonRay add-on, the
  Blender build must include both `WITH_USD` and `WITH_HYDRA`.
- MoonRay and its Hydra delegate are separate runtime dependencies; they are
  not built or bundled by these commands. A compatible delegate and its
  shared libraries must be installed and discoverable at runtime. This fork
  documents MoonRay builds for Linux and macOS, but does not provide or claim a
  native Windows MoonRay build. Blender itself can still be built on Windows.
- Some Blender release data files required by the install step (including
  startup/preview blend files, the splash image, and brush icon data) are
  absent from this fork's current source tree. If installation fails because
  one of these files is missing, restore the matching Blender 5.2 release
  assets before rebuilding; do not substitute assets from a different Blender
  version.

- [Code Review & Bug Tracker](https://projects.blender.org)
- [Developer Forum](https://devtalk.blender.org)
- [Developer Documentation](https://developer.blender.org/docs/)


License
-------

Blender as a whole is licensed under the GNU General Public License, Version 3.
Individual files may have a different but compatible license.

See [blender.org/about/license](https://www.blender.org/about/license) for details.
