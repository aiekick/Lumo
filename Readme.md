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
- [ ] RTX Rendering
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

# Nodes :

- [X] Assets / Mesh Node 
- [X] Assets / Texture 2D Node 
- [X] Breaks / Break Texture 2D Group Node
- [X] Divers / Grid / Axis Node
- [ ] Lighting / Attenuation Node
- [X] Lighting / Diffuse Node
- [X] Lighting / Light Group Node
- [ ] Lighting / MetalNess Node
- [X] Lighting / Model Shadow Node
- [ ] Lighting / RoughNess Node
- [X] Lighting / Shadow Mappings Node
- [X] Lighting / Specular Node
- [X] Modifiers / Smooth Normals Node
- [X] Outputs / Output 2D Node
- [X] Outputs / Output 3D Node
- [X] Postpro / Blur Node
- [X] Postpro / Laplacian Node
- [X] Postpro / SSAO Node
- [X] Renderers / Channels Renderer Node
- [X] Renderers / Deferred Renderer Node
- [X] Renderers / Heatmap Renderer Node
- [X] Renderers / Matcap Renderer Node
- [ ] Simulation / GrayScott Node
- [X] Utils / Depth To Pos Node
- [ ] Utils / Math Node (add, sub, mul, smoothstep, step, etc..)
- [X] Utils / Mesh Attributes Node
- [X] Utils / Pos To Depth Node
- [X] Variables / Variable Node

# ScreenShots

v0.2.798 :

My Shadow mapping (max 8 lights), diffuse, specular and SSAO
![v0_2_794](doc/screenshots/Lumo_Windows_Debug_x64_0_2_798.png)

v0.2.32 :

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

## And my libs :

- [ctools](https://github.com/aiekick/cTools.git)
- [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog.git)
- [BuildInc](https://github.com/aiekick/buildinc.git)
