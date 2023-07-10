cmake_minimum_required(VERSION 3.1)

set(PROJECT Gaia)

enable_language(C CXX)

option(USE_SHARED_LIB_GAIA  "Enable Gaia Shared Lib" OFF)

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

file(GLOB_RECURSE PROJECT_INC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX include FILES ${PROJECT_INC_RECURSE})

file(GLOB IMGUI_SRC_FILES 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/misc/freetype/imgui_freetype.cpp)
source_group(TREE ${CMAKE_SOURCE_DIR}/3rdparty/imgui/ PREFIX src FILES ${IMGUI_SRC_FILES})

file(GLOB IMGUI_INC_FILES
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/misc/freetype/imgui_freetype.h
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.h)
source_group(TREE ${CMAKE_SOURCE_DIR}/3rdparty/imgui/ PREFIX include FILES ${IMGUI_INC_FILES})

if(UNIX)
	if(APPLE)
		add_definitions(-DAPPLE)
	else()
		add_definitions(-DLINUX)
	endif()
	add_definitions(-DUNIX)
elseif(WIN32)
	add_definitions(-DWIN32)
	if(MINGW)
		add_definitions(-DMINGW32)
	endif()
endif()

if(MSVC)
	add_definitions(-D__STDC__)
	add_definitions(-DMSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-DNOMINMAX) ## for avoid issue regarding std::max and std::min
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic") ## -std=gnu++0x
endif ()

find_package(Vulkan REQUIRED)
add_definitions(-DVULKAN)

## Vulkan HPP, default dispatcher
add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

## for prevent cast issues betwwen vk:: and VK if compiling a 32 bits version
add_definitions(-DVULKAN_HPP_TYPESAFE_CONVERSION=1) 

if (USE_SHARED_LIB_GAIA)
	set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
	set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
	add_library(${PROJECT} ${PROJECT_SRC_RECURSE} ${PROJECT_INC_RECURSE} ${IMGUI_SRC_FILES} ${IMGUI_INC_FILES})
	target_compile_definitions(${PROJECT} INTERFACE BUILD_SHARED_LIBS)
	set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
	set_target_properties(${PROJECT} PROPERTIES POSITION_INDEPENDENT_CODE ON)
else()
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
	set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
	add_library(${PROJECT} STATIC ${PROJECT_SRC_RECURSE} ${PROJECT_INC_RECURSE} ${IMGUI_SRC_FILES} ${IMGUI_INC_FILES})
endif()

target_link_libraries(${PROJECT} 
	${Vulkan_LIBRARIES}
	${UTYPES_LIBRARIES}
	${TRACY_LIBRARIES}
	${LODEPNG_LIBRARIES}
	${IMGUI_LIBRARIES}
	${CTOOLS_LIBRARIES}
	${GLSLANG_LIBRARIES}
	${FREETYPE_LIBRARIES}
)
target_include_directories(${PROJECT} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${Vulkan_INCLUDE_DIRS}
	${CTOOLS_INCLUDE_DIR}
	${GLSLANG_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIR}
)

set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)

set(${PROJECT}_Output_Name "${PROJECT}$<$<CONFIG:Debug>:d>")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_Output_Name}")

set(GAIA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
set(GAIA_LIBRARIES ${PROJECT} PARENT_SCOPE)
set(GAIA_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

