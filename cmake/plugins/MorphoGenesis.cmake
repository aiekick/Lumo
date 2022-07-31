set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Plugins/MorphoGenesis)

set_target_properties(MorphoGenesis PROPERTIES FOLDER Plugins)

set_target_properties(MorphoGenesis PROPERTIES OUTPUT_NAME "MorphoGenesis_${CMAKE_SYSTEM_NAME}$<$<CONFIG:Debug>:_Debug>$<$<CONFIG:Release>:_Release>$<$<CONFIG:MinSizeRel>:_MinSizeRel>$<$<CONFIG:RelWithDebInfo>:_RelWithDebInfo>_${ARCH}")

set_target_properties(MorphoGenesis PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Plugins/MorphoGenesis/src)

if (USE_PLUGIN_STATIC_LINKING)
set(PROJECT_PLUGINS ${PROJECT_PLUGINS} MorphoGenesis)
endif()


