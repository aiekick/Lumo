# Lumo
Lumo is a realtime rendering presentation / lab software for 3D models

# why this name ?
Lumo is "Light" in Esperanto, its a short and pretty word to me :)

# Goals :

* Implement all realtime rendering Algo used in Games / realtime Presentation softwares
* Mastering all theses tech for learning purposes
* Implement my own idea ( by ex related to SubSurface Scatering, or Shadows )
* Implement a modern use of 3D vulkan Api, and Node Graph related stuff
* Can Render mesh's or sdf's in the same way
* And finally can export rendering to picture, video, and maybe a self little portable binary form to be used everywhere
* Implement some specific VFX algo by plugigns like (compute based mesh sim, morphogenesis, sdf meshing)
* all these algos at max as possible in GPU only (include mesh generation)

at least, support all rendering features and result of the SketchFab Renderer (yes im loving it haha)

# Gettings Started 

- Create a new project (automatically saved when you quit the soft for the moment for easier dev)
- Click right in the graph and add nodes.
- Left mouse double click on a texture slot (Orange) of nodes for show the result in the 3d viewport
- Middle mouse double click on a texture slot (Orange) of nodes for show the result in the 2d viewport
- select a node and you can tune it in the tuning pane

## to note :
 the resize events are propagated from the head of the graph (the head is the texture slots you selected)
 so when you just select a texture slot from a already created node, maybe you need to reisze the 3d viewport for make it updated.
 (to fix in few version, the resize system will be improved, but its not my priority, for the moment)

thats all

# How to build

## Build dependencies

* Vulkan SDK 1.2 at least : [latest](https://vulkan.lunarg.com/) and a compatible GPU
* Python 3.7 at least : [latest](https://www.python.org/downloads/)

## Cmake

You need to use cMake. 
You can use the gui (my prefered way).
but, if you run cmake by command line, For the 3 Os (Win, Linux, MacOs), the cMake usage is exactly the same,

    Choose a build directory. (called here my_build_directory for instance) and
    Choose a Build Mode : "Release" / "MinSizeRel" / "RelWithDebInfo" / "Debug" (called here BuildMode for instance)
    Run cMake in console : (the first for generate cmake build files, the second for build the binary)

cmake -B my_build_directory -DCMAKE_BUILD_TYPE=BuildMode
cmake --build my_build_directory --config BuildMode

Some cMake version need Build mode define via the directive CMAKE_BUILD_TYPE or via --Config when we launch the build. This is why i put the boths possibilities

## Platforms Support

all libs and code used here are cross platform, so compatible (win, linux). 

The mac platform is not supported for the moment, due to the vulkan driver. 

Could be tested at some stage with the Molten-VK implementation.. 

But i not tested it for the moment, So i know he compile fine on Windows with MSVC or clang, for the moment that's all.

# Tech's to implement :

| Published | Feature | status |
| - | - | - |
| :black_square_button: | Geometry Shader | ![img](https://progress-bar.dev/60) |
| :black_square_button: | Tesselation Shader | ![img](https://progress-bar.dev/50) |
| :white_square_button: | Compute Shader | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Vulkan Framework | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Mesh Use | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Mesh Texturing | ![img](https://progress-bar.dev/50) |
| :white_square_button: | Plugin system | ![img](https://progress-bar.dev/90) |
| :black_square_button: | Sdf Merging | ![img](https://progress-bar.dev/80) |
| :white_square_button: | Node Graph | ![img](https://progress-bar.dev/80) |
| :black_square_button: | Sub Node Graph | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Instancing | ![img](https://progress-bar.dev/0) |
| :white_square_button: | SkyBox (Cubemap, LongLat) | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Cell Shading | ![img](https://progress-bar.dev/100) |
| :black_square_button: | PBR Shading | ![img](https://progress-bar.dev/20) |
| :white_square_button: | Shadow Mapping | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Deferred Rendering | ![img](https://progress-bar.dev/100) |
| :black_square_button: | GLTF Rendering | ![img](https://progress-bar.dev/0) |
| :white_square_button: | RTX Rendering | ![img](https://progress-bar.dev/100) |
| :white_square_button: | RTX : Raygen Shader | ![img](https://progress-bar.dev/100) |
| :white_square_button: | RTX : Any hit Shader | ![img](https://progress-bar.dev/100) |
| :white_square_button: | RTX : Closest hit Shader | ![img](https://progress-bar.dev/100) |
| :white_square_button: | RTX : Miss Shader | ![img](https://progress-bar.dev/100) |
| :black_square_button: | RTX : Intersection Shader | ![img](https://progress-bar.dev/00) |
| :black_square_button: | RTX : Callable Shader | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Normal Mapping | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Parallax Mapping | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Alpha Mapping | ![img](https://progress-bar.dev/0) |
| :white_square_button: | Environment Mapping | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Light Directionnal | ![img](https://progress-bar.dev/80) |
| :black_square_button: | Light Point | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Light Spot | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Light Area | ![img](https://progress-bar.dev/0) |
| :white_square_button: | Bloom | ![img](https://progress-bar.dev/100) |
| :white_square_button: | SSAO | ![img](https://progress-bar.dev/100) |
| :black_square_button: | SSS | ![img](https://progress-bar.dev/50) |
| :black_square_button: | VR | ![img](https://progress-bar.dev/20) |
| :black_square_button: | Particles System | ![img](https://progress-bar.dev/50) |

# Plugin Nodes :

| Published | Plugin | status |
| - | - | - |
| :white_square_button: | Core Plugin | ![img](https://progress-bar.dev/60) |
| :black_square_button: | MeshSim Plugin | ![img](https://progress-bar.dev/100) |
| :black_square_button: | MeshSSS Plugin | ![img](https://progress-bar.dev/30) |
| :black_square_button: | MorphoGenesis Plugin | ![img](https://progress-bar.dev/30) |
| :black_square_button: | VDBTools Plugin | ![img](https://progress-bar.dev/20) |
| :white_square_button: | Particles Plugin | ![img](https://progress-bar.dev/50) |
| :white_square_button: | RTX Plugin | ![img](https://progress-bar.dev/60) |
| :black_square_button: | SdfMesher Plugin | ![img](https://progress-bar.dev/50) |
| :black_square_button: | Smoke Plugin | ![img](https://progress-bar.dev/30) |
| :black_square_button: | Voxel Plugin | ![img](https://progress-bar.dev/20) |
| :black_square_button: | VR Plugin | ![img](https://progress-bar.dev/70) |

## Core Plugin :

| Published | Node | status |
| - | - | - |
| :white_square_button: | Assets / Mesh Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Assets / Texture 2D Node  | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Assets / Cube Map Node  | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Breaks / Break Texture 2D Group Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Differential Operators / Laplacian Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Differential Operators / Divergence Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Differential Operators / Gradient Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Differential Operators / Curl Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Exporters / Export Texture to file Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Exporters / Export Cube Map to files Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Exporters / Export Mesh to file Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Exporters / Export Mesh to file Node (Transform feedback) | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Joins / Join Texture 2D Group Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Lighting / Fog Node | ![img](https://progress-bar.dev/0) |
| :white_square_button: | Lighting / Diffuse Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Lighting / Light Group Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Lighting / Refraction Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Lighting / Reflection Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Lighting / Model Shadow Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Lighting / RoughNess Node | ![img](https://progress-bar.dev/0) |
| :white_square_button: | Lighting / Shadow Mappings Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Lighting / Specular Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Lighting / Cell Shding Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Mesh Ops / Extrusion along path Node | ![img](https://progress-bar.dev/0) |
| :white_square_button: | Misc / 2D Layering Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Misc / Grid / Axis Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Modifiers / Smooth Normals Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Postpro / Blur Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Postpro / Bloom Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Postpro / SSAO Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Postpro / Tone Map Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Postpro / Vignette Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Primitives / UV Sphere Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Primitives / Ico Sphere Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Primitives / Cube Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Primitives / Torus Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Primitives / Parametric Absolute Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Primitives / Parametric Differential Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Primitives / Parametric UV Surface Node | ![img](https://progress-bar.dev/0) |
| :white_square_button: | Primitives / Fibonacci Ball Node | ![img](https://progress-bar.dev/50) |
| :white_square_button: | Preview / Cube Map Preview Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Preview / LongLat Preview Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / Channels Renderer Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / Deferred Renderer Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / Heatmap Renderer Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / Matcap Renderer Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / PBR Renderer Node | ![img](https://progress-bar.dev/10) |
| :white_square_button: | Renderers / Model Renderer Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / Billboard Renderer Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Simulation / GrayScott Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Simulation / Conway Node (Game of Life) | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Utils / Depth To Pos Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Utils / Math Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Utils / Mesh Attributes Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Utils / Pos To Depth Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Utils / Cube Map to LongLat Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Utils / Cube Map To MatCap Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Utils / LongLat to Cube Map Node  | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Utils / LongLat To MatCap Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Utils / Flat Gradient Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Widgets / Color Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Widgets / Variable Node | ![img](https://progress-bar.dev/30) |

## MeshSim Plugin : 

| Published | Node | status |
| - | - | - |
| :black_square_button: | Lighting / Shadow Map Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Modifiers / Simulator Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Renderers / Renderer Node | ![img](https://progress-bar.dev/100) |

## MeshSSS Plugin : 

| Published | Node | status |
| - | - | - |

## MorphoGenesis Plugin : 

| Published | Node | status |
| - | - | - |

## Particles Plugin :

| Published | Node | status |
| - | - | - |
| :white_square_button: | Emitters / Point Emitter Node | ![img](https://progress-bar.dev/20) |
| :black_square_button: | Simulation / Simulation Node | ![img](https://progress-bar.dev/0) |
| :black_square_button: | Constraints / Force Node | ![img](https://progress-bar.dev/0) |

## RTX Plugin :

| Published | Node | status |
| - | - | - |
| :white_square_button: | Builders / Model to Accel Structure Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / Model Shadow Node | ![img](https://progress-bar.dev/100) |
| :white_square_button: | Renderers / PBR Node | ![img](https://progress-bar.dev/100) |
| :black_square_button: | Renderers / SSS Node | ![img](https://progress-bar.dev/20) |

## SdfMesher Plugin : 

| Published | Node | status |
| - | - | - |

## Smoke Plugin : 

| Published | Node | status |
| - | - | - |

## VDBTools Plugin : 

| Published | Node | status |
| - | - | - |

## Voxel Plugin : 

| Published | Node | status |
| - | - | - |

## VR Plugin : 

| Published | Node | status |
| - | - | - |

# ScreenShots

v0.3.802 (Vulkan 1.2) :

A bloom effect implemented whith nodes 
![v0_3_802](doc/screenshots/Lumo_Windows_Debug_x64_0_3_802.png)

v0.2.798 (Vulkan 1.0) :

My Shadow mapping (max 8 lights), diffuse, specular and SSAO
![v0_2_794](doc/screenshots/Lumo_Windows_Debug_x64_0_2_798.png)

 v0.2.32 (Vulkan 1.0) :

My MeshSim plugin, with a basic deferred renderrer, one light shadow + ssao
![v0_2_32](doc/screenshots/Lumo_Windows_Debug_x64_0_2_32.png)

# Libraries used by Lumo :

- [Glslang](https://github.com/KhronosGroup/glslang.git) (BSD-3)
- [Glfw](https://github.com/glfw/glfw.git) (Zlib)
- [Tinyxml2](https://github.com/leethomason/tinyxml2.git) (Zlib)
- [Efsw](https://github.com/SpartanJ/efsw) (MIT)
- [Glm](https://github.com/g-truc/glm.git) (GLM)
- [Stb](https://github.com/nothings/stb.git) (MIT/Public)
- [Dear ImGui](https://github.com/ocornut/imgui) (MIT)
- [Freetype2](https://github.com/freetype/freetype2.git) (Freetype)
- [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit) (MIT)
- [LodePng](https://github.com/lvandeve/lodepng.git) (Zlib)
- [VulkanHPP](https://github.com/KhronosGroup/Vulkan-Hpp.git) (Apache 2.0)
- [ImGuiMarkDown](https://github.com/juliettef/imgui_markdown) (Zlib)
- [AlphaNum](http://davekoelle.com/alphanum.html) (MIT) [github mirror](https://github.com/aiekick/alphanum.git)
- [Cpp_Ipc](https://github.com/mutouyun/cpp-ipc) (MIT)
- [imgui_node_editor](https://github.com/thedmd/imgui-node-editor) (MIT)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) (MIT)
- [assimp](https://github.com/assimp/assimp) (ASSIMP)
- [zlib](https://github.com/madler/zlib.git) (Zlib)
- [taskflow](https://github.com/taskflow/taskflow) (MIT)
- [Tracy](https://github.com/wolfpld/tracy.git) (BSD-3)
- [glsl-tone-map](https://github.com/dmnsgn/glsl-tone-map) (MIT)

## And my libs :

- [ctools](https://github.com/aiekick/cTools.git) (MIT)
- [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog.git) (MIT)
- [BuildInc](https://github.com/aiekick/buildinc.git) (MIT)
