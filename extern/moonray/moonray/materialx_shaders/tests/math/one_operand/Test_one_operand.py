# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project
from pxr import Gf, Kind, Usd, Sdf, UsdGeom, UsdShade
import materialx_shaders.shaders as shaders
import materialx_shaders.test_utils as utils

(stage,xformPrim) = utils.createTestStage()

index=-1

#######################
# Start of floor tests
#######################

#
# ND_floor_integer
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.7,7.7,7.7), color_B=(3.3,3.3,3.3))
floored = shaders.create_ND_floor_integer(stage, material,'my_floor_integer', inAttr=checkerboard)
converter = shaders.create_TestInputsMap(stage,material,'myconverter',inInt=floored,select="RawInt")
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract', in1=checkerboard, in2=converter)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_floor_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,8.3,6.2), color_B=(3.4,1.2,5.7))
floored = shaders.create_ND_floor_float(stage, material,'my_floor_float', inAttr=checkerboard)
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract', in1=checkerboard, in2=floored)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_floor_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,8.3,6.2), color_B=(3.4,1.2,5.7))
floored = shaders.create_ND_floor_vector2(stage, material,'my_floor_vector2', inAttr=checkerboard)
texture = shaders.create_ND_subtract_vector2(stage,material,'my_subtract', in1=checkerboard, in2=floored)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_floor_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(3.4,1.2,5.7))
floored= shaders.create_ND_floor_vector3(stage, material,'my_floor_vector3', inAttr=checkerboard)
texture = shaders.create_ND_subtract_vector3(stage,material,'my_subtract', in1=checkerboard, in2=floored)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_floor_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(3.4,1.2,5.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
floored = shaders.create_ND_floor_vector4(stage, material,'my_floor_vector4', inAttr=converted)
texture = shaders.create_ND_subtract_vector4(stage,material,'my_subtract', in1=converted, in2=floored)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_floor_color3
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(3.4,1.2,5.7))
floored = shaders.create_ND_floor_color3(stage, material,'my_floor_color3', inAttr=checkerboard)
texture = shaders.create_ND_subtract_color3(stage,material,'my_subtract', in1=checkerboard, in2=floored)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_floor_color4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(3.4,1.2,5.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
floored = shaders.create_ND_floor_color4(stage, material,'my_floor_color4', inAttr=converted)
texture = shaders.create_ND_subtract_color4(stage,material,'my_subtract', in1=converted, in2=floored)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of ceil tests
######################
utils.nextRow()

#
# ND_ceil_integer
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.7,7.7,7.7), color_B=(3.3,3.3,3.3))
ceiled = shaders.create_ND_ceil_integer(stage, material,'my_ceil_integer', inAttr=checkerboard)
converter = shaders.create_TestInputsMap(stage,material,'myconverter',inInt=ceiled,select="RawInt")
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract', in1=converter, in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ceil_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,8.3,6.2), color_B=(3.8,1.2,5.7))
ceiled = shaders.create_ND_ceil_float(stage, material,'my_ceil_float', inAttr=checkerboard)
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract', in1=ceiled, in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ceil_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,8.3,6.2), color_B=(3.8,1.2,5.7))
ceiled = shaders.create_ND_ceil_vector2(stage, material,'my_ceil_vector2', inAttr=checkerboard)
texture = shaders.create_ND_subtract_vector2(stage,material,'my_subtract', in1=ceiled, in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ceil_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
ceiled= shaders.create_ND_ceil_vector3(stage, material,'my_ceil_vector3', inAttr=checkerboard)
texture = shaders.create_ND_subtract_vector3(stage,material,'my_subtract', in1=ceiled, in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ceil_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
ceiled = shaders.create_ND_ceil_vector4(stage, material,'my_ceil_vector4', inAttr=converted)
texture = shaders.create_ND_subtract_vector4(stage,material,'my_subtract', in1=ceiled, in2=converted)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ceil_color3
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
ceiled = shaders.create_ND_ceil_color3(stage, material,'my_ceil_color3', inAttr=checkerboard)
texture = shaders.create_ND_subtract_color3(stage,material,'my_subtract', in1=ceiled, in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ceil_color4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
ceiled = shaders.create_ND_ceil_color4(stage, material,'my_ceil_color4', inAttr=converted)
texture = shaders.create_ND_subtract_color4(stage,material,'my_subtract', in1=ceiled, in2=converted)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#######################
# Start of round tests
#######################
utils.nextRow()

#
# ND_round_integer
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.7,7.7,7.7), color_B=(3.3,3.3,3.3))
rounded = shaders.create_ND_round_integer(stage, material,'my_round_integer', inAttr=checkerboard)
converter = shaders.create_TestInputsMap(stage,material,'myconverter',inInt=rounded,select="RawInt")
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract', in1=converter, in2=checkerboard)
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract2', in1=texture, in2=-0.5)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_round_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,8.3,6.2), color_B=(3.8,1.2,5.7))
rounded = shaders.create_ND_round_float(stage, material,'my_round_float', inAttr=checkerboard)
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract', in1=rounded, in2=checkerboard)
texture = shaders.create_ND_subtract_float(stage,material,'my_subtract2', in1=texture, in2=-0.5)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_round_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,8.3,6.2), color_B=(3.8,1.2,5.7))
rounded = shaders.create_ND_round_vector2(stage, material,'my_round_vector2', inAttr=checkerboard)
texture = shaders.create_ND_subtract_vector2(stage,material,'my_subtract', in1=rounded, in2=checkerboard)
texture = shaders.create_ND_subtract_vector2(stage,material,'my_subtract2', in1=texture, in2=(-0.5,-0.5))
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_round_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
rounded= shaders.create_ND_round_vector3(stage, material,'my_round_vector3', inAttr=checkerboard)
texture = shaders.create_ND_subtract_vector3(stage,material,'my_subtract', in1=rounded, in2=checkerboard)
texture = shaders.create_ND_subtract_vector3(stage,material,'my_subtract2', in1=texture, in2=(-0.5,-0.5,-0.5))
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_round_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
rounded = shaders.create_ND_round_vector4(stage, material,'my_round_vector4', inAttr=converted)
texture = shaders.create_ND_subtract_vector4(stage,material,'my_subtract', in1=rounded, in2=converted)
texture = shaders.create_ND_subtract_vector4(stage,material,'my_subtract2', in1=texture, in2=(-0.5,-0.5,-0.5,-0.5))
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_round_color3
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
rounded = shaders.create_ND_round_color3(stage, material,'my_round_color3', inAttr=checkerboard)
texture = shaders.create_ND_subtract_color3(stage,material,'my_subtract', in1=rounded, in2=checkerboard)
texture = shaders.create_ND_subtract_color3(stage,material,'my_subtract2', in1=texture, in2=(-0.5,-0.5,-0.5))
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_round_color4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0.7,6.2), color_B=(3.8,1.2,5.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
rounded = shaders.create_ND_round_color4(stage, material,'my_round_color4', inAttr=converted)
texture = shaders.create_ND_subtract_color4(stage,material,'my_subtract', in1=rounded, in2=converted)
texture = shaders.create_ND_subtract_color4(stage,material,'my_subtract2', in1=texture, in2=(-0.5,-0.5,-0.5,-0.5))
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

########################
# Start of absval tests
########################
utils.nextRow()

#
# ND_absval_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.5,0.3,0.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_absval_float(stage, material,'my_absval_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_absval_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.5,0.3,0.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_absval_vector2(stage, material,'my_absval_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_absval_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_absval_vector3(stage, material,'my_absval_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_absval_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_absval_vector4(stage, material,'my_absval_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_absval_color3
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_absval_color3(stage, material,'my_absval_color3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_absval_color4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_absval_color4(stage, material,'my_absval_color4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of sign tests
######################
utils.nextRow()

#
# ND_sign_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.5,0.3,0.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_sign_float(stage, material,'my_sign_float', inAttr=checkerboard)
texture = shaders.create_ND_multiply_float(stage, material,'my_multiply',in1=texture,in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sign_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(0.5,0.3,0.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_sign_vector2(stage, material,'my_sign_vector2', inAttr=checkerboard)
texture = shaders.create_ND_multiply_vector2(stage, material,'my_multiply',in1=texture,in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sign_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_sign_vector3(stage, material,'my_sign_vector3', inAttr=checkerboard)
texture = shaders.create_ND_multiply_vector3(stage, material,'my_multiply',in1=texture,in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sign_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
texture = shaders.create_ND_sign_vector4(stage, material,'my_sign_vector4', inAttr=converted)
texture = shaders.create_ND_multiply_vector4(stage, material,'my_multiply',in1=texture,in2=converted)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sign_color3
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
texture = shaders.create_ND_sign_color3(stage, material,'my_sign_color3', inAttr=checkerboard)
texture = shaders.create_ND_multiply_color3(stage, material,'my_multiply',in1=texture,in2=checkerboard)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sign_color4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
checkerboard = shaders.create_CheckerboardMap(stage,material,'checkerboard1', num_u_tiles=2, num_v_tiles=2, color_A=(7.5,0,6.2), color_B=(-0.4,-0.2,-0.7))
# convert Color to Rgba
converted = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=checkerboard)
texture = shaders.create_ND_sign_color4(stage, material,'my_sign_color4', inAttr=converted)
texture = shaders.create_ND_multiply_color4(stage, material,'my_multiply',in1=texture,in2=converted)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of sin tests
######################
utils.nextRow()

#
# ND_sin_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
texture = shaders.create_ND_sin_float(stage, material,'my_sin_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sin_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
texture = shaders.create_ND_sin_vector2(stage, material,'my_sin_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sin_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
texture = shaders.create_ND_sin_vector3(stage, material,'my_sin_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sin_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_sin_vector4(stage, material,'my_sin_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of cos tests
######################
utils.nextRow()

#
# ND_cos_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
texture = shaders.create_ND_cos_float(stage, material,'my_cos_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_cos_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
texture = shaders.create_ND_cos_vector2(stage, material,'my_cos_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_cos_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
texture = shaders.create_ND_cos_vector3(stage, material,'my_cos_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_cos_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,1.0,2.0), valuebl=(0.0,1.0,2.0), valuetr=(12.0,13.0,14.0), valuebr=(12.0,13.0,14.0))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_cos_vector4(stage, material,'my_cos_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of tan tests
######################
utils.nextRow()

#
# ND_tan_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.2,1.2,1.2), valuebr=(1.2,1.2,1.2))
texture = shaders.create_ND_tan_float(stage, material,'my_tan_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_tan_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.2,1.2,1.2), valuebr=(1.2,1.2,1.2))
texture = shaders.create_ND_tan_vector2(stage, material,'my_tan_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_tan_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.2,1.2,1.2), valuebr=(1.2,1.2,1.2))
texture = shaders.create_ND_tan_vector3(stage, material,'my_tan_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_tan_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.2,1.2,1.2), valuebr=(1.2,1.2,1.2))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_tan_vector4(stage, material,'my_tan_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of asin tests
######################
utils.nextRow()

#
# ND_asin_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
texture = shaders.create_ND_asin_float(stage, material,'my_asin_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_asin_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
texture = shaders.create_ND_asin_vector2(stage, material,'my_asin_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_asin_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
texture = shaders.create_ND_asin_vector3(stage, material,'my_asin_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_asin_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_asin_vector4(stage, material,'my_asin_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of acos tests
######################
utils.nextRow()

#
# ND_acos_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
texture = shaders.create_ND_acos_float(stage, material,'my_acos_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_acos_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
texture = shaders.create_ND_acos_vector2(stage, material,'my_acos_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_acos_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
texture = shaders.create_ND_acos_vector3(stage, material,'my_acos_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_acos_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(1.0,1.0,1.0), valuebr=(1.0,1.0,1.0))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_acos_vector4(stage, material,'my_acos_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of sqrt tests
######################
utils.nextRow()

#
# ND_sqrt_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(0.5,0.5,0.5), valuebr=(0.5,0.5,0.5))
texture = shaders.create_ND_sqrt_float(stage, material,'my_sqrt_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sqrt_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(0.5,0.5,0.5), valuebr=(0.5,0.5,0.5))
texture = shaders.create_ND_sqrt_vector2(stage, material,'my_sqrt_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sqrt_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(0.5,0.5,0.5), valuebr=(0.5,0.5,0.5))
texture = shaders.create_ND_sqrt_vector3(stage, material,'my_sqrt_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_sqrt_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(0.5,0.5,0.5), valuebr=(0.5,0.5,0.5))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_sqrt_vector4(stage, material,'my_sqrt_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

######################
# Start of ln tests
######################
utils.nextRow()

#
# ND_ln_float
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(2.7,2.7,2.7), valuebr=(2.7,2.7,2.7))
texture = shaders.create_ND_ln_float(stage, material,'my_ln_float', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ln_vector2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(2.7,2.7,2.7), valuebr=(2.7,2.7,2.7))
texture = shaders.create_ND_ln_vector2(stage, material,'my_ln_vector2', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ln_vector3
#
# test Checkerboard -> Color -> Map -> Bool -> Map
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(2.7,2.7,2.7), valuebr=(2.7,2.7,2.7))
texture = shaders.create_ND_ln_vector3(stage, material,'my_ln_vector3', inAttr=texture)
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# ND_ln_vector4
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)
texture = shaders.create_ND_ramp4_color3(stage,material,'ramp1', valuetl=(0.0,0.0,0.0), valuebl=(0.0,0.0,0.0), valuetr=(2.7,2.7,2.7), valuebr=(2.7,2.7,2.7))
# convert Color to Rgba
texture = shaders.create_TestVec4fOutputMap(stage, material,'convertToRgba1', inAttr=texture)
texture = shaders.create_ND_ln_vector4(stage, material,'my_ln_vector4', inAttr=texture)
# convert the Rgba to a Color
texture = shaders.create_TestInputsMap(stage, material,'convertToColor', inRgba=texture, select="Rgba")
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive,material,geometry)

#
# save off a .usda file
#
stage.Save()
