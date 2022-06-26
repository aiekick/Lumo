# Lumo
Lumo is a realtime rendering presentation software for 3D models and complex scenes

# why this name ?
Lumo is "Light" in Esperanto, its a short and pretty word to me :)

# Goals :

* Implement all realtime rendering Algo used in Games / realtime Presentation softwares
* Mastering all theses tech for learning purposes
* Implement my own idea ( by ex related to SubSurface Scatering )
* Implement a modern use of 3D vulkan Api, and generic Scene Graph related stuff
* Can Render mesh's or sdf's in the same way
* And finally can export rendering to picture, video, and maybe a self little portable binary form to be used everywhere
* Implement some sepcific VFX algo by plugigns like (compute based mesh sim, morphogenesis, sdf meshing)

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

# Particular Nodes to do alone :

- [x] Shadow Map [MESH] => from light and Mesh 
- [x] Shadow [QUAD] => from pos and shadow map
- [x] Blur 2D [QUAD]
- [ ] Diffuse [QUAD] => from pos and light
- [ ] Specular [QUAD] => from pos and light
- [ ] Attenuation [QUAD] => from pos and light
- [ ] ADD [QUAD] => tex + tex
- [ ] MUL [QUAD] => tex * tex
- [ ] SUB [QUAD] => tex - tex

# ScreenShots

v0.2.32 :

My MeshSim plugin, with a basic deferred renderrer, one light shadow + ssao
![v0_2_32](doc/screenshots/Lumo_Windows_Debug_x64_0_2_32.png)
