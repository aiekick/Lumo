
## plugins

if(USE_PLUGIN_MESH_SIM)
	include(cmake/plugins/MeshSim.cmake)
endif()

if(USE_PLUGIN_MESH_SSS)
	include(cmake/plugins/MeshSSS.cmake)
endif()

if(USE_PLUGIN_SDF_MEHER)
	include(cmake/plugins/SdfMesher.cmake)
endif()

if(USE_PLUGIN_MORPHOGENESIS)
	include(cmake/plugins/MorphoGenesis.cmake)
endif()
