## plugins

if(USE_PLUGIN_CORE)
	include(cmake/plugins/Core.cmake)
endif()

if(USE_PLUGIN_AUDIART)
	include(cmake/plugins/AudiArt.cmake)
endif()

if(USE_PLUGIN_MESH_GEN)
	include(cmake/plugins/MeshGen.cmake)
endif()

if(USE_PLUGIN_MESH_SIM)
	include(cmake/plugins/MeshSim.cmake)
endif()

if(USE_PLUGIN_PLANET_SYSTEM)
	include(cmake/plugins/PlanetSystem.cmake)
endif()

if(USE_PLUGIN_RTX)
	include(cmake/plugins/RTX.cmake)
endif()

if(USE_PLUGIN_PARTICLES)
	include(cmake/plugins/Particles.cmake)
endif()

if(USE_MY_PLUGINS)
	if(USE_PLUGIN_MESH_SSS)
		include(cmake/plugins/MeshSSS.cmake)
	endif()

	if(USE_PLUGIN_SDF_MESHER)
		include(cmake/plugins/BluePrints.cmake)
		include(cmake/plugins/SdfMesher.cmake)
	endif()

	if(USE_PLUGIN_MORPHOGENESIS)
		include(cmake/plugins/MorphoGenesis.cmake)
	endif()

	if(USE_PLUGIN_SMOKE)
		include(cmake/plugins/Smoke.cmake)
	endif()

	if(USE_PLUGIN_VR)
		include(cmake/plugins/VR.cmake)
	endif()
endif()
