cmake_minimum_required(VERSION 3.1)

set(PROJECT RTX)

enable_language(C CXX)

project(${PROJECT} CXX)

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

## include(cmake/3rdparty.cmake)

set_target_properties(uninstall PROPERTIES FOLDER "cmakeTargets")

find_package(Vulkan REQUIRED)

## Vulkan HPP, default dispatcher
add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

## for prevent cast issues betwwen vk:: and VK if compiling a 32 bits version
add_definitions(-DVULKAN_HPP_TYPESAFE_CONVERSION=1) 

## other define
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../src/Gui/CustomImGuiFileDialogConfig.h")
add_definitions(-DUSE_IMGUI="../imgui/imgui.h")
add_definitions(-DPROJECT_NAME="${PROJECT}")
add_definitions(-DUSE_STD_FILESYSTEM)
add_definitions(-DUSE_VULKAN)

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
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

set(LINKING_TYPE MODULE)
if (USE_PLUGIN_STATIC_LINKING)
	set(LINKING_TYPE STATIC)
	add_definitions(-DUSE_PLUGIN_STATIC_LINKING)
endif()

add_library(${PROJECT} ${LINKING_TYPE}
	${CMAKE_RECURSE}
	${SHADER_SOURCES}
	${IMGUIIMPL_SOURCES}
	${PROJECT_SRC_RECURSE}
	${IMGUI_FREETYPE_SOURCES}
	${PROJECT_SHADERS_RECURSE}
)

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()

## set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}")

set(FINAL_BIN_DIR ${CMAKE_BINARY_DIR}/../bin)
message("Plugin ${PROJECT} => ${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES	LIBRARY_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")

message("BACKEND_INCLUDE_DIRS : ${Vulkan_INCLUDE_DIRS}")
message("BACKEND_LIBRARIES : ${Vulkan_LIBRARIES}")

set(CMAKE_INSTALL_PREFIX ${FINAL_BIN_DIR}/plugins)
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT ${PROJECT}_PLUGINS)

include_directories(
	src
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/libs/Common/src
	${CMAKE_SOURCE_DIR}/libs
	${CMAKE_SOURCE_DIR}/3rdparty
	${CMAKE_SOURCE_DIR}/3rdparty/glm
	${CMAKE_SOURCE_DIR}/3rdparty/glslang
	${CMAKE_SOURCE_DIR}/3rdparty/assimp/include
	${EFSW_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${PLOG_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${CPPIPC_INCLUDE_DIR}
	${LFWATCH_INCLUDE_DIR}
	${Vulkan_INCLUDE_DIRS}
	${TINYXML2_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIR}
	${IMGUIZMO_INCLUDE_DIR}
	${IMGUIFILEDIALOG_INCLUDE_DIR}
	${IMGUI_NODE_EDITOR_INCLUDE_DIR}
	${COMMON_INCLUDE_DIR}
)

target_link_libraries(${PROJECT} PRIVATE
	${COMMON_LIBRARIES}
)

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

## only for Debug
add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND BuildInc -prefix=RTX 1000:10 $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/src/Headers/RTXBuild.h>
	DEPENDS ${PROJECT}
)

