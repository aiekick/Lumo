# Lumo
Lumo is a realtime rendering presentation software for 3D models and complex scenes

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

at least, support all rendering features and result of the SketchFab Renderer

# Tech's to implement :

- [ ] Geometry Shader
- [ ] Tesselation Shader
- [x] Compute Shader
- [x] Vulkan Framework
- [X] Mesh Use
- [X] Mexh Texturing
- [X] Plugin system
- [ ] Sdf Merging
- [X] Node Graph
- [ ] Instancing
- [ ] SkyBox (Cubemap, LongLat)
- [ ] Cell Shading
- [ ] PBR Shading
- [x] Shadow Mapping
- [x] Deferred Rendering
- [ ] GLTF Rendering
- [X] RTX Rendering
- [X] RTX : Raygen Shader
- [X] RTX : Any hit Shader
- [X] RTX : Closest hit Shader
- [X] RTX : Miss Shader
- [ ] RTX : Intersection Shader
- [ ] RTX : Callable Shader
- [ ] Normal Mapping
- [ ] Parallax Mapping
- [ ] Alpha Mapping
- [ ] Environment Mapping
- [x] Light Directionnal
- [ ] Light Point
- [ ] Light Spot
- [ ] Light Area
- [ ] Bloom
- [x] SSAO
- [ ] SSS
- [ ] VR
- [ ] Particles System

# Plugin Nodes :

## Core Plugin :

| Created | Node | status |
| - | - | - |
| :white_square_button: | Assets / Mesh Node | ![img](https://progress-bar.dev/90) |
|  :white_square_button: | Assets / Texture 2D Node  | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Breaks / Break Texture 2D Group Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Differential Operators / Laplacian Node | ![img](https://progress-bar.dev/100) |
|  :black_square_button: | Lighting / Attenuation Node | ![img](https://progress-bar.dev/0) |
|  :white_square_button: | Lighting / Diffuse Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Lighting / Light Group Node | ![img](https://progress-bar.dev/50) |
|  :black_square_button: | Lighting / MetalNess Node | ![img](https://progress-bar.dev/0) |
|  :white_square_button: | Lighting / Model Shadow Node | ![img](https://progress-bar.dev/100) |
|  :black_square_button: | Lighting / RoughNess Node | ![img](https://progress-bar.dev/0) |
|  :white_square_button: | Lighting / Shadow Mappings Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Lighting / Specular Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Misc / Grid / Axis Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Misc / 2D Layering | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Modifiers / Smooth Normals Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Postpro / Blur Node (WIP) | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Postpro / SSAO Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Postpro / Tone Map Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Renderers / Channels Renderer Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Renderers / Deferred Renderer Node (WIP) | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Renderers / Heatmap Renderer Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Renderers / Matcap Renderer Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Renderers / PBR Renderer Node (WIP) | ![img](https://progress-bar.dev/10) |
|  :white_square_button: | Simulation / GrayScott Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Utils / Depth To Pos Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Utils / Math Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Utils / Mesh Attributes Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Utils / Pos To Depth Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Widgets / Color Node | ![img](https://progress-bar.dev/100) |
|  :white_square_button: | Widgets / Variable Node | ![img](https://progress-bar.dev/20) |

## RTX Plugin :

- [X] Builders / Model to Accel Structure Node (Accel Structure Builder)
- [X] Renderers / PBR Node (WIP)
- [X] Renderers / SSS Node (WIP)

## Particles Plugin :

- [X] Emitters / Mesh Emitter Node (WIP)
- [ ] Emitters / Mesh Fur Node (WIP)
- [X] Primitives / Fibonacci Ball Node (WIP)
- [ ] Renderers / Billboard Renderer Node (WIP)
- [X] Renderers / Point Renderer Node (WIP)
- [ ] Renderers / Mesh Fur Renderer Node (WIP)
- [X] Simulation / Simulation Node (WIP)
- [ ] Simulation / Mesh Fur Simulation Node (WIP)
- [ ] Constraints / Force Node

# Gettings Started 

- Create a new project (automatically saved when you quit the soft for the moment for easier dev)
- Click right in the graph and add nodes.
- Left mouse double click on a texture slot (Orange) of nodes for show the result in the 3d viewport
- Middle mouse double click on a texture slot (Orange) of nodes for show the result in the 2d viewport
- select a node and you can tune it in the tuning pane

thats all

# ScreenShots

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
- [VulkanHPP](https://github.com/KhronosGroup/Vulkan-Hpp.git)
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
