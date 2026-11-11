# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project
from pxr import Gf, Kind, Usd, Sdf, UsdGeom, UsdShade
import materialx_shaders.shaders as shaders
import materialx_shaders.test_utils as utils

(stage,xformPrim) = utils.createTestStage()

index=-1

#
# ND_atan2_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.8,1.0,1.2), color_B=(0.4,0.5,0.6))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(1.0,2.0,3.0), color_B=(4.0,2.0,0.5))
texture = shaders.create_ND_atan2_float(stage, material,'my_maxfloat', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_atan2_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.8,1.0,1.2), color_B=(0.4,0.5,0.6))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(1.0,2.0,3.0), color_B=(4.0,2.0,0.5))
texture = shaders.create_ND_atan2_vector2(stage, material,'my_maxvector2', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_atan2_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.8,1.0,1.2), color_B=(0.4,0.5,0.6))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(1.0,2.0,3.0), color_B=(4.0,2.0,0.5))
texture = shaders.create_ND_atan2_vector3(stage, material,'my_maxvector3', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_atan2_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.8,1.0,1.2), color_B=(0.4,0.5,0.6))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(1.0,2.0,3.0), color_B=(4.0,2.0,0.5))
# convert Color to Rgba
texture1 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture1)
texture2 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba2', inAttr=texture2)
texture = shaders.create_ND_atan2_vector4(stage, material,'my_maxvector4', in1=texture1, in2=texture2)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# save off a .usda file
#
stage.Save()
