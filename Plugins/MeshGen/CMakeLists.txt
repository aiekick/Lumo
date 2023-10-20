cmake_minimum_required(VERSION 3.1)

set(PROJECT MeshGen)

enable_language(C CXX)

project(${PROJECT} CXX)

option(USE_PLUGIN_MESH_GEN "Enable the plugin MeshGen" OFF)

if (USE_PLUGIN_MESH_GEN)

set(LLVM_USE_CRT_DEBUG MDd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MD CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "")

## for group smake targets in the dir cmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmakeTargets")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

add_definitions(-DUSE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS) ## for the resize imgui issue when we have child glfw windows

include(cmake/tinyexpr.cmake)

if (uninstall)
	set_target_properties(uninstall PROPERTIES FOLDER "cmakeTargets")
endif()

## other define
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_LUMO_BACKEND_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomLumoBackendConfig.h")
add_definitions(-DCUSTOM_IMWIDGETS_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomImWidgetsConfig.h")
add_definitions(-DUSE_IMGUI="${CMAKE_SOURCE_DIR}/libs/Gaia/3rdparty/ImGuiPack/3rdparty/imgui/imgui.h") # for use ImGuiPack
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomImGuiFileDialogConfig.h")
add_definitions(-DPROJECT_NAME="${PROJECT}")
add_definitions(-DUSE_STD_FILESYSTEM)
add_definitions(-DUSE_VULKAN)

add_definitions(-DUSE_IM_TOOLS)
add_definitions(-DUSE_IM_GUIZMO)
add_definitions(-DUSE_IMCOOL_BAR)
add_definitions(-DUSE_IMGUI_MARKDOW)
add_definitions(-DUSE_IMGUI_FILE_DIALOG)
add_definitions(-DUSE_IMGUI_NODE_EDITOR)

if(UNIX)
    add_definitions(-DUNIX)
elseif(WIN32)
    add_definitions(-DWIN32)
endif()

if(MSVC)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
	add_definitions(-DNOMINMAX) ## for avoid issue regarding std::max and std::min
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter -Wno-unknown-pragmas")
endif ()

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

file(GLOB SHADER_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.geom
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.tesseval
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.tessctrl
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp)
source_group(Shaders FILES ${SHADER_SOURCES})

file(GLOB_RECURSE PROJECT_SHADERS_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/builtins/*.glsl 
	${CMAKE_CURRENT_SOURCE_DIR}/builtins/*.scen 
	${CMAKE_CURRENT_SOURCE_DIR}/builtins/*.comp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Shaders FILES ${PROJECT_SHADERS_RECURSE})

file(GLOB_RECURSE CMAKE_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/cmake PREFIX cmake FILES ${CMAKE_RECURSE})

set(ENABLE_EXPORTS ON CACHE STRING "" FORCE)

add_library(${PROJECT} MODULE
	${CMAKE_RECURSE}
	${SHADER_SOURCES}
	${PROJECT_SRC_RECURSE}
	${PROJECT_SHADERS_RECURSE}
)

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()

## set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}")
target_compile_definitions(${PROJECT} PRIVATE BUILD_GAIA_SHARED_LIBS)	
target_compile_definitions(${PROJECT} PRIVATE BUILD_CTOOLS_SHARED_LIBS)	
target_compile_definitions(${PROJECT} PRIVATE BUILD_IMGUI_PACK_SHARED_LIBS)
target_compile_definitions(${PROJECT} PRIVATE BUILD_LUMO_BACKEND_SHARED_LIBS)

if(USE_DEBUG_SANITIZER)
	target_compile_options(${PROJECT} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -static-libasan>)
	target_link_options(${PROJECT} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -static-libasan>)
	message("Address Sanitizer enabled for projet ${PROJECT}")
endif()

set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin/plugins)
message("Plugin ${PROJECT} => ${FINAL_BIN_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}/debug")
set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PLUGIN_PREFIX}_${PROJECT}_${CMAKE_SYSTEM_NAME}_$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	LIBRARY_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES FOLDER Lumo_Plugins)

install(IMPORTED_RUNTIME_ARTIFACTS ${PROJECT} DESTINATION plugins COMPONENT PLUGIN_BINARY_${PROJECT})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
	${LUMO_BACKEND_INCLUDE_DIRS}
	${TINYEXPR_INCLUDE_DIR}
	${TINYEXPRCPP_INCLUDE_DIR}
)

target_link_libraries(${PROJECT}
	${LUMO_BACKEND_LIBRARIES}
	${TINYEXPR_LIBRARIES}
	${TINYEXPRCPP_LIBRARIES}
)

# add plugin name in parent LOADED_LUMO_PLUGINS list
set(LOADED_LUMO_PLUGINS ${LOADED_LUMO_PLUGINS} ${PROJECT} PARENT_SCOPE)

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

## only for Debug
add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND BuildInc -prefix=${PROJECT} 1000000:10 $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/src/Headers/${PROJECT}Build.h>
	DEPENDS ${PROJECT}
)

########################################################
########################################################
########################################################

endif() #USE_PLUGIN_MESH_GEN