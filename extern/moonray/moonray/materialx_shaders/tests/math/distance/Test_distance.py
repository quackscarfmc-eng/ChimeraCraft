# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project
from pxr import Gf, Kind, Usd, Sdf, UsdGeom, UsdShade
import materialx_shaders.shaders as shaders
import materialx_shaders.test_utils as utils

(stage,xformPrim) = utils.createTestStage()

index=-1

#
# ND_distance_vector2
#

#
# ND_distance_vector2 test default in1 and constant in2
#
# The first part is boilerplate to create a subgraph and material
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

texture = shaders.create_ND_distance_vector2(stage, material, 'my_distance_vector2', in2=(0.1,0.2))

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector2 test constant in1 and default in2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

texture = shaders.create_ND_distance_vector2(stage, material, 'my_distance_vector2', in1=(0.5,0.3))

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector2 test constant in1 and default in2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

checkerboard1 = shaders.create_CheckerboardMap(stage, material, 'my_checkerboard1',
                                               num_u_tiles=2, num_v_tiles=2,
                                               color_A=(0.5,0.8,0.0), color_B=(0.1,0.2,0.0)) 
checkerboard2 = shaders.create_CheckerboardMap(stage, material, 'my_checkerboard2',
                                               num_u_tiles=4, num_v_tiles=4,
                                               color_A=(0.1,0.5,0.0), color_B=(0.2,0.1,0.0)) 
texture = shaders.create_ND_distance_vector2(stage, material, 'my_distance_vector2', in1=checkerboard1, in2=checkerboard2)

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector3
#
utils.nextRow()

#
# ND_distance_vector3 test with default in1 and constant in2
#
# The first part is boilerplate to create a subgraph and material
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

texture = shaders.create_ND_distance_vector3(stage, material, 'my_distance_vector3', in2=(0.1,0.2,0.3))

# the rest of the test is boilerplate unless a trivial emissive material isn't sufficient
emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector3 test with constant in1 and default in2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

texture = shaders.create_ND_distance_vector3(stage, material, 'my_distance_vector3', in1=(0.5,0.3,0.6))

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector3 test with bound in1 and in2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

checkerboard1 = shaders.create_CheckerboardMap(stage, material, 'my_checkerboard1',
                                               num_u_tiles=2, num_v_tiles=2,
                                               color_A=(0.5,0.8,0.1), color_B=(0.1,0.2,0.5)) 
checkerboard2 = shaders.create_CheckerboardMap(stage, material, 'my_checkerboard2',
                                               num_u_tiles=4, num_v_tiles=4,
                                               color_A=(0.1,0.5,0.2), color_B=(0.2,0.1,0.3)) 
texture = shaders.create_ND_distance_vector3(stage, material, 'my_distance_vector3', in1=checkerboard1, in2=checkerboard2)

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector4
#
utils.nextRow()

#
# ND_distance_vector4 test with default in1 and constant in2
#
# The first part is boilerplate to create a subgraph and material
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

texture = shaders.create_ND_distance_vector4(stage, material, 'my_distance_vector4', in2=(0.5,0.2,0.3,0.4))

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector4 test with constant in1 and default in2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

texture = shaders.create_ND_distance_vector4(stage, material, 'my_distance_vector4', in1=(0.3,0.8,0.1,0.2))

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# ND_distance_vector4 test with bound in1 and in2
#
index+=1
geometry = utils.create_MeshPlaneNext(stage, xformPrim)
material = utils.create_UniqueMaterial(stage)

checkerboard1 = shaders.create_CheckerboardMap(stage, material, 'my_checkerboard1',
                                               num_u_tiles=2, num_v_tiles=2,
                                               color_A=(0.1,0.2,0.5), color_B=(0.5,0.8,0.3))
convert1 = shaders.create_ND_convert_vector3_vector4(stage, material, 'my_convert1', inAttr=checkerboard1)
checkerboard2 = shaders.create_CheckerboardMap(stage, material, 'my_checkerboard2',
                                               num_u_tiles=4, num_v_tiles=4,
                                               color_A=(0.7,0.1,0.3), color_B=(0.1, 0.6,0.7)) 
convert2 = shaders.create_ND_convert_vector3_vector4(stage, material, 'my_convert2', inAttr=checkerboard2)
texture = shaders.create_ND_distance_vector4(stage, material, 'my_distance_vector4', in1=convert1, in2=convert2)

emissive = shaders.create_DwaEmissiveMaterial(stage, material, 'MyMaterialShader', emission=texture)
utils.bindMaterialToGeom(emissive, material, geometry)

#
# save off a .usda file
#
stage.Save()
