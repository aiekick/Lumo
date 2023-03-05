## plugins

if(USE_PLUGIN_CORE)
	include(cmake/plugins/Core.cmake)
	add_definitions(-DUSE_PLUGIN_CORE)
endif()

if(USE_PLUGIN_AUDIART)
	include(cmake/plugins/AudiArt.cmake)
	add_definitions(-DUSE_PLUGIN_AUDIART)
endif()

if(USE_PLUGIN_MESH_GEN)
	include(cmake/plugins/MeshGen.cmake)
	add_definitions(-DUSE_PLUGIN_MESH_GEN)
endif()

if(USE_PLUGIN_MESH_SIM)
	include(cmake/plugins/MeshSim.cmake)
	add_definitions(-DUSE_PLUGIN_MESH_SIM)
endif()

if(USE_PLUGIN_MESH_SSS)
	include(cmake/plugins/MeshSSS.cmake)
	add_definitions(-DUSE_PLUGIN_MESH_SSS)
endif()

if(USE_PLUGIN_PLANET)
	include(cmake/plugins/Planet.cmake)
	add_definitions(-DUSE_PLUGIN_PLANET)
endif()

if(USE_PLUGIN_SDF_MESHER)
	include(cmake/plugins/SdfMesher.cmake)
	add_definitions(-DUSE_PLUGIN_SDF_MESHER)
endif()

if(USE_PLUGIN_SO_GLSL)
	include(cmake/plugins/SoGLSL.cmake)
	add_definitions(-DUSE_PLUGIN_SO_GLSL)
endif()


if(USE_PLUGIN_MORPHOGENESIS)
	include(cmake/plugins/MorphoGenesis.cmake)
	add_definitions(-DUSE_PLUGIN_MORPHOGENESIS)
endif()

if(USE_PLUGIN_RTX)
	include(cmake/plugins/RTX.cmake)
	add_definitions(-DUSE_PLUGIN_RTX)
endif()

if(USE_PLUGIN_PARTICLES)
	include(cmake/plugins/Particles.cmake)
	add_definitions(-DUSE_PLUGIN_PARTICLES)
endif()

if(USE_PLUGIN_SMOKE)
	include(cmake/plugins/Smoke.cmake)
	add_definitions(-DUSE_PLUGIN_SMOKE)
endif()

if(USE_PLUGIN_VR)
	include(cmake/plugins/VR.cmake)
	add_definitions(-DUSE_PLUGIN_VR)
endif()