## plugins

if(USE_PLUGIN_MESH_SIM)
	include(cmake/plugins/MeshSim.cmake)
	add_definitions(-DUSE_PLUGIN_MESH_SIM)
endif()

if(USE_PLUGIN_MESH_SSS)
	include(cmake/plugins/MeshSSS.cmake)
	add_definitions(-DUSE_PLUGIN_MESH_SSS)
endif()

if(USE_PLUGIN_SDF_MESHER)
	include(cmake/plugins/SdfMesher.cmake)
	add_definitions(-DUSE_PLUGIN_SDF_MESHER)
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
