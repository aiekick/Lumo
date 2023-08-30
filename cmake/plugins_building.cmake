## plugins

if(USE_PLUGIN_CORE)
	add_dependencies(${PROJECT} Core)
endif()

if(USE_PLUGIN_AUDIART)
	add_dependencies(${PROJECT} AudiArt)
endif()

if(USE_PLUGIN_MESH_GEN)
	add_dependencies(${PROJECT} MeshGen)
endif()

if(USE_PLUGIN_MESH_SIM)
	add_dependencies(${PROJECT} MeshSim)
endif()

if(USE_PLUGIN_PLANET_SYSTEM)
	add_dependencies(${PROJECT} PlanetSystem)
endif()

if(USE_PLUGIN_RTX)
	add_dependencies(${PROJECT} RTX)
endif()

if(USE_PLUGIN_PARTICLES)
	add_dependencies(${PROJECT} Particles)
endif()

if(USE_MY_PLUGINS)
	if(USE_PLUGIN_MESH_SSS)
		add_dependencies(${PROJECT} MeshSSS)
	endif()

	if(USE_PLUGIN_SDF_MESHER)
		add_dependencies(${PROJECT} BluePrints)
		add_dependencies(${PROJECT} SdfMesher)
	endif()

	if(USE_PLUGIN_MORPHOGENESIS)
		add_dependencies(${PROJECT} MorphoGenesis)
	endif()

	if(USE_PLUGIN_SMOKE)
		add_dependencies(${PROJECT} Smoke)
	endif()

	if(USE_PLUGIN_VR)
		add_dependencies(${PROJECT} VR)
	endif()
endif()