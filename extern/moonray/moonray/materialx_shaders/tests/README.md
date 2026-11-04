# materialx\_shaders.test\_utils module

This provides convenience functions for creating a scene

**(stage,xformPrim) \= utils.createTestStage()**

stage is the main USD handle for a scene being created.  
**xformPrim** is a **UsdGeomXform** object which geometry can be hung off of for transforming into the for the scene. The default is no rotation, translation or scale which should usually be fine for a simple grid of texture tests. In the case of needing a transform, use standard UsdShade functions to update the **UsdGeomXform**. Normally **xformPrim** can be passed to geometry creation as is.

For the common case of having a grid of tests in a scene to, for instance, test all of the variants of a shader or default, uniform, or varying inputs there are some convenient functions. **space** can be an explicit USD path string but more likely it will just be the xformPrim returned from

**geometry \= create\_MeshCubeNext(stage, space)**  
Create a cube at the current grid location and advance the current location to the right  
**geometry \= create\_MeshPlaneNext(stage, space)**  
Create a plane at the current grid location and advance the current location to the right  
**nextColumn()**  
Advance to the next grid location without creating geometry. This should only be needed for creating a gap between the test cases.  
**nextRow()**  
Start a new row of tests

For more explicit control of placement there are lower level functions

**create\_MeshCube(stage, space, name, size, x, y, z)**  
**create\_MeshPlane(stage, space, name, size, x, y, z)**  
Explicitly create a cube or plane at the 3D location in the provided space. **space** can be an explicit USD path string but more likely it will just be the xformPrim returned from **createTestState()**

# materialx\_shaders.shaders module

This provides functions for creating USD shading nodes for any of the shaders in moonray.

The functions are in the form

**shaders.create\_\<shadername\>(**  
**stage,**  
**\<namespace\>,**  
**\<nodename\>,**  
**\<input\>=\<value,**  
**…)**

**stage** is what's returned from **createTestStage()**  
**namespace** can be an explicit USD path string or a USD object which has a path  
**nodename** is a unique name for the node in the namespace  
**input** is optional and can be a series of name value pairs for the inputs for the shaders  
if a particular input is omitted then the default value will be used  
if a the input is assigned a value then the uniform value will be used  
if the input is a USD shader node object then there will be a shader binding  
Due to collision with python reserved words two common input names are renamed  
in \-\> inAttr  
type \-\> typeAttr

\# A Simple header that can probably be used by most tests  
from pxr import Gf, Kind, Usd, Sdf, UsdGeom, UsdShade  
import materialx\_shaders.shaders as shaders  
import materialx\_shaders.test\_utils as utils

\# Create stage and xform for the test  
(stage,xformPrim) \= utils.createTestStage()

\# Create geometry to shade and a material  
geometry \= utils.create\_MeshPlaneNext(stage, xformPrim)  
testName \= '/MyMaterialTest'+str(index)  
material \= UsdShade.Material.Define(stage,testName)

\# Create an ND\_distance\_vector2 texture with in1 as the  
\# default value and in2 set to a constant  
\# This will tend to be where the real test development happens  
texture \= shaders.create\_ND\_distance\_vector2(  
stage, material, 'my\_distance\_vector2',  
in2=(0.1,0.2))

\# Create a trivial material to make the texture visible and bind it to geometry  
\# This will likely be the same for every test unless the shader being tested needs  
\# a special material or needs to interact with lighting  
\# An emissive material provides the simplest pass through of the the texture values  
emissive \= shaders.create\_DwaEmissiveMaterial(stage,  
                                              material,  
                                              'MyMaterialShader',  
                                              emission=texture)  
material.CreateSurfaceOutput().ConnectToSource(emissive.ConnectableAPI(), "surface")  
UsdShade.MaterialBindingAPI(geometry).Bind(material) 

\# Save the result at the end  
\# The name will be test.usda or the name provided  
\# with \-o or \--output on the command line of this script  
stage.Save()

## Creating map networks

When creating map networks texture objects are passed for input values instead of a constant  
checkerboard1 \= shaders.create\_CheckerboardMap(stage, material, 'my\_checkerboard1',  
                                               num\_u\_tiles=2, num\_v\_tiles=2,  
                                               color\_A=(0.5,0.8,0.0), color\_B=(0.1,0.2,0.0))   
checkerboard2 \= shaders.create\_CheckerboardMap(stage, material, 'my\_checkerboard2',  
                                               num\_u\_tiles=4, num\_v\_tiles=4,  
                                               color\_A=(0.1,0.5,0.0), color\_B=(0.2,0.1,0.0))   
texture \= shaders.create\_ND\_distance\_vector2(stage, material, 'my\_distance\_vector2', in1=checkerboard1, in2=checkerboard2)

## USD Python Constant Formats

Assigning constants to shader inputs in the create function is fairly obvious for everything but matrices.

Bool/boolean		**inBool=True**  
Integer	/integer	**inInt=16711935**  
String/string		**inString="Hello"**  
Float/float		**inFloat=0.5**  
Vec2f/vector2		**inVec2f=(0.9,0.1)**  
Vec3f/vector3		**inVec3f=(0.3,0.6,0.9)**  
Vec4f/vector4		**inVect4f=(0.1,0.2,0.3,0.4)**  
Color/color3		**inColor=(1.0,1.0,1.0)**  
Rgba/color4		**inRgba=(1.0,0.0,1.0,1.0)**  
Mat3f/matrix33	**inMat3f=Gf.Matrix3d(1.0,0.0,0.0,**  
                    **0.0,0.0,0.0,**  
                    **0.0,0.0,1.0)**  
Mat4f/matrix44	**inMat4f=Gf.Matrix4d(1.0,0.0,0.0,0.0,**  
                    **0.0,1.0,0.0,0.0,**  
                    **0.0,0.0,1.0,0.0,**  
                    **0.0,0.0,0.0,1.0)**

# Causing the Script to be a Test

In CMakeLists.txt file

add\_pats\_test(  
    SCRIPT testname.py  
    CANONICALS scene.exr  
    RENDER\_ARGS \-size 600 800  
)