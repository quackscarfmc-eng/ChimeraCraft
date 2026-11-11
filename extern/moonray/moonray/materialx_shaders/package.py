# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project

# -*- coding: utf-8 -*-
import os, sys

unittestflags = (['@run_all', '--unittest-xml', '--no-cache']
                 if os.environ.get('BROKEN_CUSTOM_ARGS_UNITTESTS') else [])

name = 'materialx_shaders'

if 'early' not in locals() or not callable(early):
    def early(): return lambda x: x

@early()
def version():
    """
    Increment the build in the version.
    """
    _version = '2.0'
    from rezbuild import earlybind
    return earlybind.version(this, _version)

description = 'MaterialX Shaders for Moonray'

authors = [
    'PSW Rendering and Shading',
    'moonbase-dev@dreamworks.com',
]

help = ('For assistance, '
        "please contact the folio's owner at: moonbase-dev@dreamworks.com")

build_system = 'cmake'
build_system_pbr = 'cmake_modules'

variants = [
    [   # variant 0
        'os-rocky-9',
        'opt_level-optdebug',
        'refplat-vfx2023.1',
        'openimageio-2.4.8.0.x',
        'gcc-11.x',
        'openvdb-11'
    ],
    [   # variant 1
        'os-rocky-9',
        'opt_level-debug',
        'refplat-vfx2023.1',
        'openimageio-2.4.8.0.x',
        'gcc-11.x',
        'openvdb-11'
    ],
    [   # variant 2
        'os-rocky-9',
        'opt_level-optdebug',
        'refplat-vfx2023.1',
        'openimageio-2.4.8.0.x',
        'clang-17.0.6.x',
        'openvdb-10' # openvdb-11 requires llvm-15 which conflicts with clang-17's llvm-17, so it stays at openvdb-10.
    ],
    [   # variant 3
        'os-rocky-9',
        'opt_level-optdebug',
        'refplat-vfx2024.0',
        'openimageio-2.4.8.0.x',
        'gcc-11.x',
        'openvdb-11',
        'python-3.11'
    ],
    [   # variant 4
       'os-rocky-9',
       'opt_level-optdebug',
       'refplat-vfx2025.0',
       'openimageio-3.0',
       'gcc-11.x',
       'openvdb-12',
       'python-3.11'
    ],
    [   # variant 5
        'os-rocky-9',
        'opt_level-optdebug',
        'refplat-houdini21.0',
        'openimageio-3.0',
        'gcc-11.x',
        'openvdb-12'
    ],
    [   # variant 6
        'os-rocky-9',
        'opt_level-optdebug',
        'refplat-vfx2022.0',
        'openimageio-2.3.20.0.x',
        'gcc-9.3.x.1',
        'openvdb-9',
        'zlib-1.2.11.x',
        'opensubdiv-3.5.0.x.0'
    ],
]

conf_rats_variants = variants[0:2]
conf_CI_variants = list(filter(lambda v: 'os-CentOS-7' in v, variants))

requires = [
    'moonray-18.10',
    'scene_rdl2-16.6'
]

private_build_requires = [
    build_system_pbr,
    'cmake_modules-2.0',
    'cppunit',
    'ispc-1.20.0.x',
    'materialx', # for cmd/make_stub_shader
    'python',
    'orats',
    'hdMoonray',
    'moonshine'
]

commandstr = lambda i: "cd build/"+os.path.join(*variants[i])+"; ctest -j $(nproc)"
testentry = lambda i: ("variant%d" % i,
                       { "command": commandstr(i),
                         "requires": ["cmake-3.23"] + variants[i] } )
testlist = [testentry(i) for i in range(len(variants))]
tests = dict(testlist)


def commands():
    prependenv('CMAKE_MODULE_PATH', '{root}/lib64/cmake')
    prependenv('CMAKE_PREFIX_PATH', '{root}')
    prependenv('SOFTMAP_PATH', '{root}')
    prependenv('MOONRAY_DSO_PATH', '{root}/rdl2dso')
    prependenv('RDL2_DSO_PATH', '{root}/rdl2dso')
    prependenv('LD_LIBRARY_PATH', '{root}/lib64')
    prependenv('PATH', '{root}/bin')
    prependenv('MOONRAY_CLASS_PATH', '{root}/coredata')
    prependenv('PYTHONPATH', '{root}/python/lib/$PYTHON_NAME')


uuid = '6f68ec78-0086-4d2a-93ec-635b490ff899'

config_version = 0
