// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

// This plugin is a pure C++ Sdr parser plugin. It is discovered and loaded by
// USD's PlugRegistry through plugInfo.json; there is no pxr.MoonrayShaderParser
// Python module (the CMake target has Python::Module commented out).
//
// A TfScriptModuleLoader::RegisterLibrary() call used to live here and
// registered the *bogus* Python module name "pxr.MoonrayShaderParser". USD
// then tried to import that module at plugin load and emitted:
//   Warning: Error loading lib moonrayShaderParser's module
//            'pxr.MoonrayShaderParser': ModuleNotFoundError.
// Do not re-introduce a TfScriptModuleLoader registration unless a real Python
// binding module is built and shipped alongside this library.

#include "pxr/pxr.h"

PXR_NAMESPACE_OPEN_SCOPE

// Intentionally no TF_REGISTRY_FUNCTION(TfScriptModuleLoader) registration.

PXR_NAMESPACE_CLOSE_SCOPE
