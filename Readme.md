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

- [X] Assets / Mesh Node 
- [X] Assets / Texture 2D Node 
- [X] Breaks / Break Texture 2D Group Node
- [X] Differential Operators / Laplacian Node
- [ ] Lighting / Attenuation Node
- [X] Lighting / Diffuse Node
- [X] Lighting / Light Group Node
- [ ] Lighting / MetalNess Node
- [X] Lighting / Model Shadow Node
- [ ] Lighting / RoughNess Node
- [X] Lighting / Shadow Mappings Node
- [X] Lighting / Specular Node
- [X] Misc / Grid / Axis Node
- [X] Misc / 2D Layering
- [X] Modifiers / Smooth Normals Node
- [X] Postpro / Blur Node (WIP)
- [X] Postpro / SSAO Node
- [X] Postpro / Tone Map Node
- [X] Renderers / Channels Renderer Node
- [X] Renderers / Deferred Renderer Node (WIP)
- [X] Renderers / Heatmap Renderer Node
- [X] Renderers / Matcap Renderer Node
- [ ] Renderers / PBR Renderer Node (WIP)
- [X] Simulation / GrayScott Node
- [X] Utils / Depth To Pos Node
- [X] Utils / Math Node (add, sub, mul, smoothstep, step, etc..)
- [X] Utils / Mesh Attributes Node
- [X] Utils / Pos To Depth Node
- [X] Widgets / Color Node
- [X] Widgets / Variable Node

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
