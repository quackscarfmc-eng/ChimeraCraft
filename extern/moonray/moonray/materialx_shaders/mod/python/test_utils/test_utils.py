# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the Moonray Project
import pxr
import argparse
from pxr import Usd, UsdGeom, UsdShade

gsize = 0.13
gstartx = -1.2
gx = gstartx
gy = 1.7
gw = (gsize*2)*1.2
gh = (gsize*2)*1.2
geomIndex = 0
materialIndex = 0

def HandleShaderAttribute(shader, attrName, attrValue, attrType, bindable=False):
    if isinstance(attrValue, pxr.UsdShade.Shader):
        if bindable:
            shader.CreateInput(attrName, attrType).ConnectToSource(attrValue.ConnectableAPI(), 'out')
        else:
            print('Warning: {}.{} cannot be bound (ignoring)'.format(shader.GetShaderId(),attrName))
    elif attrValue!=None:
        shader.CreateInput(attrName, attrType).Set(attrValue)

#
# extract the name of an object if one is provided and append the name
#
def HandlePath(path, name):
    # if the path is an object then convert it into a string
    if isinstance(path, pxr.Usd.SchemaBase):
        path=path.GetPath().pathString
    path = path + '/' +  name

    return path

def testParseArgs():
    parse = argparse.ArgumentParser(description='Test generation')
    parse.add_argument('--output', '-o', type=str, dest='outputFile', default='test.usda', help='Output file name')
    args = parse.parse_args()
    return args

def createTestStage(size=0.13, x=-1.2, y=1.7):
    global gsize
    global gstartx
    global gx
    global gy
    global gw
    global gh
    global geomIndex
    global materialIndex
    gsize = size
    gstartx = x
    gx = x
    gy = y
    gw = (gsize*2)*1.2
    gh = (gsize*2)*1.2
    geomIndex = 0
    materialIndex = 0
    testArgs = testParseArgs()
    stage = Usd.Stage.CreateNew(testArgs.outputFile)

    xformPrim = UsdGeom.Xform.Define(stage, '/hello')

    # set some values to make usdchecker happy
    UsdGeom.SetStageMetersPerUnit(stage, 0.01)
    UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
    stage.SetDefaultPrim(stage.GetPrimAtPath("/hello"))
    return (stage, xformPrim)

def nextColumn():
    global gx
    global geomIndex
    gx += gw
    geomIndex += 1

def nextRow():
    global gx
    global gy
    gx = gstartx
    gy -= gh

def create_MeshCube(stage, path, name, size, x, y, z):
    path=HandlePath(path,name)
    meshPrim = UsdGeom.Mesh.Define(stage, path)
    prim = stage.GetPrimAtPath(path);
    prim.ApplyAPI(UsdShade.MaterialBindingAPI)
    meshPrim.CreatePointsAttr([
        ( size+x,  size+y,  size+z),
        (-size+x,  size+y,  size+z),
        (-size+x, -size+y,  size+z),
        ( size+x, -size+y,  size+z),
        (-size+x, -size+y, -size+z),
        (-size+x,  size+y, -size+z),
        ( size+x,  size+y, -size+z),
        ( size+x, -size+y, -size+z)])
    meshPrim.CreateFaceVertexCountsAttr([4, 4, 4, 4, 4, 4])
    meshPrim.CreateFaceVertexIndicesAttr([0, 1, 2, 3, 4, 5, 6, 7, 4, 7, 3, 2, 0, 3, 7, 6, 4, 2, 1, 5, 6, 5, 1, 0])
    meshPrim.CreateExtentAttr([(-1, -1, -1), (1, 1, 1)])
    meshPrim.CreateSubdivisionSchemeAttr('none')

    return meshPrim

def create_MeshCubeNext(stage, xformPrim):
    meshPrim = create_MeshCube(stage, xformPrim, 'geom'+str(geomIndex), gsize, gx, gy, 0)

    nextColumn()

    return meshPrim

def create_MeshPlane(stage, path, name, size, x, y, z):
    path=HandlePath(path,name)
    meshPrim = UsdGeom.Mesh.Define(stage, path)
    prim = stage.GetPrimAtPath(path);
    prim.ApplyAPI(UsdShade.MaterialBindingAPI)
    meshPrim.CreatePointsAttr([
        ( size+x,  size+y,  size+z),
        (-size+x,  size+y,  size+z),
        (-size+x, -size+y,  size+z),
        ( size+x, -size+y,  size+z)])
    meshPrim.CreateFaceVertexCountsAttr([4])
    meshPrim.CreateFaceVertexIndicesAttr([0, 1, 2, 3])
    meshPrim.CreateExtentAttr([(-1, -1, -1), (1, 1, 1)])
    meshPrim.CreateSubdivisionSchemeAttr('none')

    return meshPrim

def create_MeshPlaneNext(stage, xformPrim):
    meshPrim = create_MeshPlane(stage, xformPrim, 'geom'+str(geomIndex), gsize, gx, gy, 0)

    nextColumn()

    return meshPrim

def create_MeshPlane(stage, path, name, size, x, y, z):
    path=HandlePath(path,name)
    meshPrim = UsdGeom.Mesh.Define(stage, path)
    prim = stage.GetPrimAtPath(path);
    prim.ApplyAPI(UsdShade.MaterialBindingAPI)
    meshPrim.CreatePointsAttr([
        ( size+x,  size+y,  size+z),
        (-size+x,  size+y,  size+z),
        (-size+x, -size+y,  size+z),
        ( size+x, -size+y,  size+z)])
    meshPrim.CreateFaceVertexCountsAttr([4])
    meshPrim.CreateFaceVertexIndicesAttr([0, 1, 2, 3])
    meshPrim.CreateExtentAttr([(-1, -1, -1), (1, 1, 1)])
    meshPrim.CreateSubdivisionSchemeAttr('none')

    return meshPrim

def create_MeshPlaneNext(stage, xformPrim):
    meshPrim = create_MeshPlane(stage, xformPrim, 'geom'+str(geomIndex), gsize, gx, gy, 0)

    nextColumn()

    return meshPrim

def create_UniqueMaterial(stage):
    global materialIndex
    materialPrim = UsdShade.Material.Define(stage, '/TestMaterial'+str(materialIndex))
    materialIndex += 1
    return materialPrim


def bindMaterialToGeom(surfaceShader, materialPrim, geomPrim):
    materialPrim.CreateSurfaceOutput().ConnectToSource(surfaceShader.ConnectableAPI(), 'surface')
    materialBindingAPI = UsdShade.MaterialBindingAPI(geomPrim)
    materialBindingAPI.Bind(materialPrim)
