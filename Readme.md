[![Win Lumo](https://github.com/aiekick/Lumo/actions/workflows/Win_Lumo.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Lumo.yml)
[![Linux Lumo](https://github.com/aiekick/Lumo/actions/workflows/Linux_Lumo.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Lumo.yml)

# Lumo
Lumo is a realtime lab software arount 3D models

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

thats all

# How to build

## Build dependencies

* Vulkan SDK 1.2 at least : [latest](https://vulkan.lunarg.com/) and a compatible GPU
* Python 3.7 at least : [latest](https://www.python.org/downloads/)

Succesfully Tested with Vulkan SDK 1.3.224.1

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
| :x:  | Geometry Shader | ![img](https://progress-bar.dev/60) |
| :heavy_check_mark:  | Tesselation Shader | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Compute Shader | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Vulkan Framework | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Mesh Use | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Mesh Texturing | ![img](https://progress-bar.dev/50) |
| :heavy_check_mark:  | Plugin system | ![img](https://progress-bar.dev/90) |
| :x:  | Sdf Merging | ![img](https://progress-bar.dev/80) |
| :heavy_check_mark:  | Node Graph | ![img](https://progress-bar.dev/80) |
| :x:  | Sub Node Graph | ![img](https://progress-bar.dev/0) |
| :x:  | Instancing | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | SkyBox (Cubemap, LongLat) | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Cell Shading | ![img](https://progress-bar.dev/100) |
| :x:  | PBR Shading | ![img](https://progress-bar.dev/20) |
| :heavy_check_mark:  | Shadow Mapping | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Deferred Rendering | ![img](https://progress-bar.dev/100) |
| :x:  | GLTF Rendering | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | RTX Rendering | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | RTX : Raygen Shader | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | RTX : Any hit Shader | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | RTX : Closest hit Shader | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | RTX : Miss Shader | ![img](https://progress-bar.dev/100) |
| :x:  | RTX : Intersection Shader | ![img](https://progress-bar.dev/00) |
| :x:  | RTX : Callable Shader | ![img](https://progress-bar.dev/0) |
| :x:  | Normal Mapping | ![img](https://progress-bar.dev/0) |
| :x:  | Parallax Mapping | ![img](https://progress-bar.dev/0) |
| :x:  | Alpha Mapping | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Environment Mapping | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Light Directionnal | ![img](https://progress-bar.dev/80) |
| :x:  | Light Point | ![img](https://progress-bar.dev/0) |
| :x:  | Light Spot | ![img](https://progress-bar.dev/0) |
| :x:  | Light Area | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Bloom | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | SSAO | ![img](https://progress-bar.dev/100) |
| :x:  | SSS | ![img](https://progress-bar.dev/50) |
| :x:  | VR | ![img](https://progress-bar.dev/20) |
| :x:  | Particles System | ![img](https://progress-bar.dev/50) |

# Nodes Plugins :

| Published | Plugin | Open Source | Free | status | Description |
| - | - | - | - | - | - |
| :heavy_check_mark: | AudiArt | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |
| :heavy_check_mark: | Core | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/60) | Core pack of Nodes |
| :heavy_check_mark: | MeshGen | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/60) | Procedural Mesh generation |
| :x:  | MeshSim | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/90) | Texture Less Reaction Diffusion Simulation on Meshs |
| :x:  | MeshSSS | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/30) | My Own SSS Idea For RT SSS for Static mesh with Moving Ligth |
| :x:  | MorphoGenesis | :x: | :x: | ![img](https://progress-bar.dev/30) | Real Time MorphoGenesis Algos |
| :heavy_check_mark: | Particles | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/50) | GPU Particle Engine |
| :heavy_check_mark: | Planet System | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Planet System Renderer |
| :heavy_check_mark: | RTX | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/60) | RTX Pipeline |
| :x:  | SdfMesher | :x: | :x: | ![img](https://progress-bar.dev/50) | Procedural Mesh Generation from Shader, Pictures or SDF |
| :x:  | Common | :x: | :heavy_check_mark: | ![img](https://progress-bar.dev/30) | Advanced Shader System Over GLSL (Used in NoodlesPlate) |
| :x:  | Smoke | :x: | :x: | ![img](https://progress-bar.dev/30) | GPU RT Smoke System |
| :x:  | UVDiffMap | :x: | :x: | ![img](https://progress-bar.dev/10) | Fast UV Mapping idea for any mesh Based on Vertex Diffusion |
| :x:  | VDBTools | :x: | :x: | ![img](https://progress-bar.dev/20) | OpenVDB Toolkit |
| :x:  | Voxel | :x: | :x: | ![img](https://progress-bar.dev/20) | Voxel Engine/Editor |
| :x:  | VR | :x: | :x: | ![img](https://progress-bar.dev/70) | VR Engine |

## AudiArt Plugin :

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Effects / FFT Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Operations / Historize Node  | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Sources / Speacker Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Viewers / Source Preview Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Viewers / Visu Hex Grid Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Windowing / Blackman FIlter Node  | ![img](https://progress-bar.dev/0) |

## Core Plugin :

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Assets / Mesh Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Assets / Texture 2D Node  | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Assets / Cube Map Node  | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Breaks / Break Texture 2D Group Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Differential Operators / Laplacian Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Differential Operators / Divergence Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Differential Operators / Gradient Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Differential Operators / Curl Node | ![img](https://progress-bar.dev/100) |
| :x:  | Exporters / Export Texture to file Node | ![img](https://progress-bar.dev/0) |
| :x:  | Exporters / Export Cube Map to files Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Exporters / Export Mesh to file Node | ![img](https://progress-bar.dev/100) |
| :x:  | Exporters / Export Mesh to file Node (Transform feedback) | ![img](https://progress-bar.dev/0) |
| :x:  | Joins / Join Texture 2D Group Node | ![img](https://progress-bar.dev/0) |
| :x:  | Lighting / Fog Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Lighting / Diffuse Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Lighting / Light Group Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Lighting / Refraction Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Lighting / Reflection Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Lighting / Model Shadow Node | ![img](https://progress-bar.dev/100) |
| :x:  | Lighting / RoughNess Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Lighting / Shadow Mappings Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Lighting / Specular Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Lighting / Cell Shading Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Misc / 2D Layering Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Misc / Grid / Axis Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Misc / Scene Merger Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Modifiers / Smooth Normals Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Blur Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Bloom Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / SSAO Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Tone Map Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Vignette Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Preview / Cube Map Preview Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Preview / LongLat Preview Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Channels Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Deferred Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Heatmap Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Matcap Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / PBR Renderer Node | ![img](https://progress-bar.dev/10) |
| :heavy_check_mark:  | Renderers / Model Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Billboard Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Simulation / GrayScott Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Simulation / Conway Node (Game of Life) | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Utils / Depth To Pos Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Utils / Math Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Utils / Mesh Attributes Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Utils / Pos To Depth Node | ![img](https://progress-bar.dev/100) |
| :x:  | Utils / Cube Map to LongLat Node | ![img](https://progress-bar.dev/0) |
| :x:  | Utils / Cube Map To MatCap Node | ![img](https://progress-bar.dev/0) |
| :x:  | Utils / LongLat to Cube Map Node  | ![img](https://progress-bar.dev/0) |
| :x:  | Utils / LongLat To MatCap Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Utils / Flat Gradient Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Widgets / Color Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Widgets / Variable Node | ![img](https://progress-bar.dev/30) |

## MeshGen Plugin : 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Curve / Parametric Curve Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Curve / Parametric Curve Differential Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Mesh / Primitive Node | ![img](https://progress-bar.dev/80) |
| :x:  | Mesh Ops / Extrusion along path Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Surface / Parametric UV Surface Node | ![img](https://progress-bar.dev/100) |

## MeshSim Plugin : 

| Published | Node | status |
| - | - | - |
| :x:  | Lighting / Shadow Map Node | ![img](https://progress-bar.dev/100) |
| :x:  | Modifiers / Simulator Node | ![img](https://progress-bar.dev/100) |
| :x:  | Renderers / Renderer Node | ![img](https://progress-bar.dev/100) |

## MeshSSS Plugin : 

| Published | Node | status |
| - | - | - |

## MorphoGenesis Plugin : 

| Published | Node | status |
| - | - | - |

## Particles Plugin :

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Emitters / Point Emitter Node | ![img](https://progress-bar.dev/20) |
| :x:  | Simulation / Simulation Node | ![img](https://progress-bar.dev/0) |
| :x:  | Constraints / Force Node | ![img](https://progress-bar.dev/0) |

## Planet System Plugin :

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Planet Node | ![img](https://progress-bar.dev/30) |

## RTX Plugin :

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Builders / Model to Accel Structure Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Model Shadow Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / PBR Node | ![img](https://progress-bar.dev/100) |
| :x:  | Renderers / SSS Node | ![img](https://progress-bar.dev/20) |

## SdfMesher Plugin : 

| Published | Node | status |
| - | - | - |

## Common Plugin : 

| Published | Node | status |
| - | - | - |

## Smoke Plugin : 

| Published | Node | status |
| - | - | - |

## UVDiffMap Plugin :

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
