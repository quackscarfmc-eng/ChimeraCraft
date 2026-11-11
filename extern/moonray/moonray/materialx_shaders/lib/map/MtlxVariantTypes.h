// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file MtlxVariantTypes.h

#include "ispc/MtlxVariantTypes.hh"
#include <scene_rdl2/scene/rdl2/rdl2.h>

//
// GETSAMPLEFUNC = <SHADER_NAME>_getSampleFunc
// example ND_add_float_getSampleFunc
//
#define GETSAMPLEFUNC CONCAT2(SHADER_NAME,_getSampleFunc)

//
// ATTR_NAMESPACE = <SHADER_NAME>_attr
// example ND_add_float_attr
//
#define ATTR_NAMESPACE CONCAT2(SHADER_NAME,_attr)

#define MTLX_ATTR_DECLARE_BEGIN namespace ATTR_NAMESPACE {

#define MTLX_ATTR_DECLARE_END }

#define MTLX_ATTR_DEFINE_BEGIN \
using namespace ATTR_NAMESPACE;\
extern "C" \
RDL2_DSO_EXPORT \
scene_rdl2::rdl2::SceneObjectInterface \
rdl2_declare(scene_rdl2::rdl2::SceneClass& sceneClass) \
{\
    auto rdl2_dso_interface = Map::declare(sceneClass);\
    __pragma(warning(push)) \
    __pragma(warning(disable:1711)) // #1711 is writing to a static var.

#define MTLX_ATTR_DEFINE_END \
    __pragma(warning(pop))\
    return rdl2_dso_interface;\
}

void MtlxPHPrint(int k);
