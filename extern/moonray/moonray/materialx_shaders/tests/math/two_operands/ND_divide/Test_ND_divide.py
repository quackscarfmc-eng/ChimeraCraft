# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project
from pxr import Gf, Kind, Usd, Sdf, UsdGeom, UsdShade
import materialx_shaders.shaders as shaders
import materialx_shaders.test_utils as utils

(stage,xformPrim) = utils.createTestStage()

index=-1

#
# ND_divide_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(0,3.3,0), color_B=(1.0,1.0,1.0))
texture = shaders.create_ND_divide_float(stage, material,'my_divide_float', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(1.0,3.3,0.5), color_B=(1.0,1.42,1.0))
texture = shaders.create_ND_divide_vector2(stage, material,'my_divide_vector2', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
texture = shaders.create_ND_divide_vector3(stage, material,'my_divide_vector3', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
# convert Color to Rgba
texture1 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture1)
texture2 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba2', inAttr=texture2)
texture = shaders.create_ND_divide_vector4(stage, material,'my_divide_vector4', in1=texture1, in2=texture2)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

utils.nextRow()

#
# ND_divide_color3
#
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
index+=1
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
texture = shaders.create_ND_divide_color3(stage, material,'my_divide_color3', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_color4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
# convert Color to Rgba
texture1 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture1)
texture2 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba2', inAttr=texture2)
texture = shaders.create_ND_divide_color4(stage, material,'my_divide_color4', in1=texture1, in2=texture2)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_matrix33
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
# convert Color to Mat3f
texture1 = shaders.create_TestMat3fOutputMap(stage, material,'convertToMat3f1', inAttr=texture1)
texture2 = shaders.create_TestMat3fOutputMap(stage, material,'convertToMat3f2', inAttr=texture2)
texture = shaders.create_ND_divide_matrix33(stage, material,'my_divide_matrix33', in1=texture1, in2=texture2)
# convert the Mat3f to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inMat3f=texture, select="Mat3f")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_matrix44
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
# convert Color to Mat4f
texture1 = shaders.create_TestMat4fOutputMap(stage, material,'convertToMat4f1', inAttr=texture1)
texture2 = shaders.create_TestMat4fOutputMap(stage, material,'convertToMat4f2', inAttr=texture2)
texture = shaders.create_ND_divide_matrix44(stage, material,'my_divide_matrix44', in1=texture1, in2=texture2)
# convert the Mat4f to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inMat4f=texture, select="Mat4f")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

utils.nextRow()

#
# ND_divide_vector2FA
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
texture = shaders.create_ND_divide_vector2FA(stage, material,'my_divide_vector2FA', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_vector3FA
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
texture = shaders.create_ND_divide_vector3FA(stage, material,'my_divide_vector3FA', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_vector4FA
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
# convert Color to Rgba
texture1 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture1)
texture = shaders.create_ND_divide_vector4FA(stage, material,'my_divide_vector4FA', in1=texture1, in2=texture2)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_divide_color3FA
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
texture = shaders.create_ND_divide_color3FA(stage, material,'my_divide_color3FA', in1=texture1, in2=texture2)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

utils.nextRow()

#
# ND_divide_color4FA
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture1 = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.7,1.0,0.5), color_B=(0,0,0))
texture2 = shaders.create_CheckerboardMap(stage,material,'checkerboard2', num_u_tiles=4, num_v_tiles=4, color_A=(10.0,3.3,2.0), color_B=(1.0,1.42,1.0))
# convert Color to Rgba
texture1 = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture1)
texture = shaders.create_ND_divide_color4FA(stage, material,'my_divide_color4FA', in1=texture1, in2=texture2)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# save off a .usda file
#
stage.Save()
