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

The mac platform is not supported for the moment, due to the vulkan driver. Could be tested at some stage with the Molten-VK implementation.. 

# Tech's to implement :

| Published | Feature | status |
| - | - | - |
| :x:  | Geometry Shader | ![img](https://progress-bar.dev/60) |
| :heavy_check_mark:  | Tesselation Shader | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Compute Shader | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Vulkan Framework | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Mesh Use | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Mesh Texturing | ![img](https://progress-bar.dev/50) |
| :heavy_check_mark:  | Plugin system | ![img](https://progress-bar.dev/100) |
| :x:  | Sdf Merging | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Node Graph | ![img](https://progress-bar.dev/100) |
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
| :heavy_check_mark:  | Light Directionnal | ![img](https://progress-bar.dev/100) |
| :x:  | Light Point | ![img](https://progress-bar.dev/0) |
| :x:  | Light Spot | ![img](https://progress-bar.dev/0) |
| :x:  | Light Area | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Bloom | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | SSAO | ![img](https://progress-bar.dev/100) |
| :x:  | SSS | ![img](https://progress-bar.dev/50) |
| :x:  | SSR (Reflection) | ![img](https://progress-bar.dev/80) |
| :x:  | SSR (Refraction) | ![img](https://progress-bar.dev/0) |
| :x:  | VR | ![img](https://progress-bar.dev/20) |
| :x:  | Particles System | ![img](https://progress-bar.dev/50) |
| :x:  | Depth of field | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Dilation | ![img](https://progress-bar.dev/100) |
| :heavy_check_markx:  | Blur | ![img](https://progress-bar.dev/100) |
| :heavy_check_markx:  | Bloom | ![img](https://progress-bar.dev/100) |
| :heavy_check_markx:  | Chromatic Aberration | ![img](https://progress-bar.dev/100) |
| :heavy_check_markx:  | Sharpen | ![img](https://progress-bar.dev/100) |
| :heavy_check_markx:  | Tone Mapping | ![img](https://progress-bar.dev/100) |
| :heavy_check_markx:  | Vignette | ![img](https://progress-bar.dev/100) |

# Lumo Native Nodes :

| Category | Name | status | Description |
| - | - | - | - |
| Assets/Loader | Cube Map | ![img](https://progress-bar.dev/100) | load a cube amp texture |
| Assets/Loader | Model | ![img](https://progress-bar.dev/80) | load a model |
| Assets/Loader | Texture 2D | ![img](https://progress-bar.dev/100) | Load a texture |
| Assets/Saver | Model Exporter | ![img](https://progress-bar.dev/100) | Export model to file. support sketchfab animations |
| Assets/Saver | Texture Exporter | ![img](https://progress-bar.dev/100) | Export texture to file |
| Assets/Misc | Grid | ![img](https://progress-bar.dev/100) | - |
| Assets/Misc | Scene Merger | ![img](https://progress-bar.dev/100) | Meerge many node in the same FBO target |
| Assets/Renderer | Channel Renderer | ![img](https://progress-bar.dev/100) | can display channels of each model (pos, nor, tan, btan, col) |
| Assets/Renderer | HeatMap Renderer | ![img](https://progress-bar.dev/100) | HeatMap of the color channel |
| Assets/Renderer | MatCap Renderer | ![img](https://progress-bar.dev/100) | Render a matcap on the model |
| Assets/Renderer | Model Renderer | ![img](https://progress-bar.dev/100) | - |
| Assets/Utils | Math | ![img](https://progress-bar.dev/100) | - |
| Assets/Utils | Mesh Attribute | ![img](https://progress-bar.dev/100) | extract mesh attribute (pos, nor, tan, btan, col, depth) |
| Assets/Widgets | Variable | ![img](https://progress-bar.dev/100) | - |
| Assets/Widgets | Color | ![img](https://progress-bar.dev/100) | - |

# Lumo Plugins :

## AudiArt Plugin :

### Status : 

[![Win Audiart](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Audiart.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Audiart.yml ) 
[![Linux Audiart](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Audiart.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Audiart.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Effects / FFT Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Operations / Historize Node  | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Sources / Speacker Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Viewers / Source Preview Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Viewers / Visu Hex Grid Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Windowing / Blackman FIlter Node  | ![img](https://progress-bar.dev/0) |

## BluePrints Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :x: | ![img](https://progress-bar.dev/20) | Blue Print node system like in Unreal Engine for shaders |

### Nodes: 

| Published | Node | status |
| - | - | - |

## Code Generator Plugin :

### Status : 

[![Win Code Generator](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_CodeGenerator.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_CodeGenerator.yml) 
[![Linux Code Generator](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_CodeGenerator.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_CodeGenerator.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Description : 

This plugin not expose nodes, but a pane for easy node generation ready to compile.
you can set inputs and outputs of one or many nodes.
then with one operation, generate base cades for : 
- node class
- module class
- pass class

## Lighting Plugin :

### Status : 

[![Win Lighting](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Lighting.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Lighting.yml) 
[![Linux Lighting](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Lighting.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Lighting.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Breaks / Break Texture 2D Group Node | ![img](https://progress-bar.dev/100) |
| :x:  | Exporters / Export Texture Group to file Node | ![img](https://progress-bar.dev/0) |
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
| :heavy_check_mark:  | Preview / Cube Map Preview Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Preview / LongLat Preview Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Deferred Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / PBR Renderer Node | ![img](https://progress-bar.dev/10) |
| :heavy_check_mark:  | Renderers / Billboard Renderer Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Utils / Depth Converter Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Utils / Pos To Depth Node | ![img](https://progress-bar.dev/100) |
| :x:  | Utils / Cube Map to LongLat Node | ![img](https://progress-bar.dev/0) |
| :x:  | Utils / Cube Map To MatCap Node | ![img](https://progress-bar.dev/0) |
| :x:  | Utils / LongLat to Cube Map Node  | ![img](https://progress-bar.dev/0) |
| :x:  | Utils / LongLat To MatCap Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Utils / Flat Gradient Node | ![img](https://progress-bar.dev/100) |

## MeshGen Plugin : 

### Status : 

[![Win MeshGen](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_MeshGen.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_MeshGen.yml) 
[![Linux MeshGen](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_MeshGen.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_MeshGen.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Curve / Parametric Curve Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Curve / Parametric Curve Differential Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Mesh / Primitive Node | ![img](https://progress-bar.dev/80) |
| :x:  | Mesh Ops / Extrusion along path Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Surface / Parametric UV Surface Node | ![img](https://progress-bar.dev/100) |

## MeshSim Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :x:  | Lighting / Shadow Map Node | ![img](https://progress-bar.dev/100) |
| :x:  | Modifiers / Simulator Node | ![img](https://progress-bar.dev/100) |
| :x:  | Renderers / Renderer Node | ![img](https://progress-bar.dev/100) |

## MeshSSS Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |

## Misc Plugin :

### Status : 

[![Win Misc](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Misc.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Misc.yml) 
[![Linux Misc](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Misc.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Misc.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Misc / Layering Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Misc / Sdf Texture Node | ![img](https://progress-bar.dev/30) |
| :heavy_check_mark:  | Modifiers / Smooth Normals Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Tesselation / Alien Rock Node | ![img](https://progress-bar.dev/0) |

## MorphoGenesis Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :x: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |

## Particles Plugin :

### Status : 

[![Win Particles](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Particles.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Particles.yml)
[![Linux Particles](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Particles.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Particles.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Emitters / Point Emitter Node | ![img](https://progress-bar.dev/20) |
| :x:  | Simulation / Simulation Node | ![img](https://progress-bar.dev/0) |
| :x:  | Constraints / Force Node | ![img](https://progress-bar.dev/0) |

## Planet System Plugin :

### Status : 

[![Win PlanetSystem](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_PlanetSystem.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_PlanetSystem.yml)
[![Linux PlanetSystem](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_PlanetSystem.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_PlanetSystem.yml) 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Planet Node | ![img](https://progress-bar.dev/30) |

## Post Processing Plugin :

### Status : 

[![Win Processing](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_PostProcessing.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_PostProcessing.yml) 
[![Linux Processing](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_PostProcessing.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_PostProcessing.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Postpro / Effects / Bloom Node | ![img](https://progress-bar.dev/100) 
| :heavy_check_mark:  | Postpro / Effects / Blur Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Effects / Chromatic Aberration Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Effects / Color Corrector Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Depth of Field Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Dilation Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Effects / Fog Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Grain Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / LUT Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Motion Blur Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Outlining Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Posteriztion Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Pixelisation Node | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Sharpness Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Effects / Screen Space Ambient Occlusion Node (SSAO) | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Effects / Screen Space Interior Node (SSRa) | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Screen Space Reflection Node (SSRe) | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Screen Space Refraction Node (SSRa) | ![img](https://progress-bar.dev/0) |
| :heavy_check_mark:  | Postpro / Effects / Tone Mapping Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / Effects / Vignette Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Postpro / PostProcessing Node | ![img](https://progress-bar.dev/100) |

## Simulation Plugin :

### Status : 

[![Win Simulation](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Simulation.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_Simulation.yml) 
[![Linux Simulation](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Simulation.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_Simulation.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Differential Operators / Curl Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Differential Operators / Divergence Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Differential Operators / Gradient Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Differential Operators / Laplacian Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Simulation / GrayScott Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Simulation / Conway Node (Game of Life) | ![img](https://progress-bar.dev/100) |

## RTX Plugin :

### Status : 

[![Win RTX](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_RTX.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Win_Plugin_RTX.yml)
[![Linux RTX](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_RTX.yml/badge.svg)](https://github.com/aiekick/Lumo/actions/workflows/Linux_Plugin_RTX.yml)

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/10) | Audio Manipulation for Art |

### Nodes: 

| Published | Node | status |
| - | - | - |
| :heavy_check_mark:  | Builders / Model to Accel Structure Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / Model Shadow Node | ![img](https://progress-bar.dev/100) |
| :heavy_check_mark:  | Renderers / PBR Node | ![img](https://progress-bar.dev/100) |
| :x:  | Renderers / SSS Node | ![img](https://progress-bar.dev/20) |

## SdfEditor Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :x: | ![img](https://progress-bar.dev/0) | Sdf Editor like Magica CSG |

### Nodes: 

| Published | Node | status |
| - | - | - |

## SdfMesher Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :x: | ![img](https://progress-bar.dev/40) | Sdf to Mesh Generation |

### Nodes: 

| Published | Node | status |
| - | - | - |

## Smoke Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :x: | ![img](https://progress-bar.dev/0) | Smoke Rendering like Embergen |

### Nodes: 

| Published | Node | status |
| - | - | - |

## SoGLSL Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :x: | ![img](https://progress-bar.dev/50) | Scripting Over GLSL system for write shaders |

### Nodes: 

| Published | Node | status |
| - | - | - |

## UVDiffMap Plugin :

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :x: | :x: | ![img](https://progress-bar.dev/0) | UV Mapper of high def Model based of Vertex Diffusion |

### Nodes: 

| Published | Node | status |
| - | - | - |

## VDBTools Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/0) | OpenVDB Nodes |

### Nodes: 

| Published | Node | status |
| - | - | - |

## Voxel Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/0) | Voxel Manipulation like Magicavoxel |

### Nodes: 

| Published | Node | status |
| - | - | - |

## VR Plugin : 

### Status : 

| Published | Open Source | Free | status | Description |
| - | - | - | - | - |
| :x: | :heavy_check_mark: | :heavy_check_mark: | ![img](https://progress-bar.dev/0) | Virtual Reality integration |

### Nodes: 

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
