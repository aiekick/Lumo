cmake_minimum_required(VERSION 3.1)

set(PROJECT Common)

project(${PROJECT} CXX)

enable_language(C CXX)

set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

file(GLOB IMGUI_FREETYPE_SOURCES
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/misc/freetype/imgui_freetype.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/misc/freetype/imgui_freetype.h)
source_group(src\\ImGui_FreeType FILES ${IMGUI_FREETYPE_SOURCES})

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
    # Ignore 4055 for glad
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /IGNORE:4055")
	add_definitions(-D__STDC__)
	add_definitions(-DMSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-DNOMINMAX) ## for avoid issue regarding std::max and std::min
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11") ## -std=gnu++0x
endif ()

find_package(Vulkan REQUIRED)
add_definitions(-DVULKAN)

#set(BUILD_SHARED_LIBS ON)
#if (BUILD_SHARED_LIBS)
#	add_library(${PROJECT} SHARED ${PROJECT_SRC_RECURSE} ${IMGUI_FREETYPE_SOURCES})
#	target_compile_definitions(${PROJECT} INTERFACE BUILD_SHARED_LIBS)
#	set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
#	set_target_properties(${PROJECT} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#else()
	add_library(${PROJECT} STATIC ${PROJECT_SRC_RECURSE} ${IMGUI_FREETYPE_SOURCES})
#endif()

target_include_directories(${PROJECT} PUBLIC
	src
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
)

target_link_libraries(${PROJECT} PRIVATE
	${GLFW_LIBRARIES}
	${EFSW_LIBRARIES}
	${IMGUI_LIBRARIES}
	${UTYPES_LIBRARIES}
	${ASSIMP_LIBRARIES}
	${CTOOLS_LIBRARIES}
	${CPPIPC_LIBRARIES}
	${LODEPNG_LIBRARIES}
	${GLSLANG_LIBRARIES}
	${LFWATCH_LIBRARIES}
	${IMGUIZMO_LIBRARIES}
	${TINYXML2_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${FONTICONS_LIBRARIES}
	${IMWIDGETS_LIBRARIES}
	${VKFRAMEWORK_LIBRARIES}
	${INTERFACEBLOCK_LIBRARIES}
	${IMGUIFILEDIALOG_LIBRARIES}
	${IMGUI_NODE_EDITOR_LIBRARIES}
	${IMGUICOLORTEXTEDIT_LIBRARIES}
)

set(${PROJECT}_Output_Name "${PROJECT}_${CMAKE_SYSTEM_NAME}$<$<CONFIG:Debug>:_Debug>$<$<CONFIG:Release>:_Release>$<$<CONFIG:MinSizeRel>:_MinSizeRel>$<$<CONFIG:RelWithDebInfo>:_RelWithDebInfo>_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_Output_Name}")

set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)

set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(COMMON_LIBRARIES ${PROJECT} PARENT_SCOPE)
set(COMMON_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND BuildInc -prefix=Common 1000:10 $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/src/Common/CommonBuild.h>
	DEPENDS ${PROJECT}
)