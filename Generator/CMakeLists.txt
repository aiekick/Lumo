cmake_minimum_required(VERSION 3.1)

set(PROJECT LumoCodeGenerator)

enable_language(C CXX)

project(${PROJECT} CXX)

set(LLVM_USE_CRT_DEBUG MDd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MD CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

## for group smake targets in the dir cmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmakeTargets")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_PREFIX "${FINAL_BIN_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}/debug")

set(USE_SHARED_LIBS ON CACHE BOOL "" FORCE)

set_target_properties(uninstall PROPERTIES FOLDER "cmakeTargets")

find_package(Vulkan REQUIRED)

add_definitions(-DUSE_GLFW3)
add_definitions(-DUSE_STD_FILESYSTEM)
add_definitions(-DIMGUI_ENABLE_FREETYPE)
add_definitions(-DPROJECT_NAME="${PROJECT}")
add_definitions(-DVMA_VULKAN_VERSION=1002000)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DPROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DUSE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS) ## for the resize imgui issue when we have child glfw windows
add_definitions(-DSTB_IMAGE_INCLUDE="${CMAKE_SOURCE_DIR}/3rdparty/stb/stb_image.h")
add_definitions(-DSTB_IMAGE_RESIZE_INCLUDE="${CMAKE_SOURCE_DIR}/3rdparty/stb/stb_image_resize.h")
add_definitions(-DSTB_IMAGE_WRITE_INCLUDE="${CMAKE_SOURCE_DIR}/3rdparty/stb/stb_image_write.h")
add_definitions(-DCUSTOM_IMWIDGETS_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomImWidgetsConfig.h")
add_definitions(-DUSE_IMGUI="${CMAKE_SOURCE_DIR}/libs/Gaia/3rdparty/ImGuiPack/3rdparty/imgui/imgui.h") # for use ImGuiPack
add_definitions(-DCUSTOM_LUMO_BACKEND_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomLumoBackendConfig.h")
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomImGuiFileDialogConfig.h")

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /IGNORE:4055 /std:c++17")
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
    add_definitions(-DGLFW_EXPOSE_NATIVE_WIN32)
	add_definitions(-DNOMINMAX) ## for avoid issue regarding std::max and std::min
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17 -Wunused-parameter")
endif ()

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX _debug)
endif()

file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(main FILES ${PROJECT_MAIN})

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

file(GLOB IMGUIIMPL_SOURCES 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.h)
source_group(src\\ImGui_Impl FILES ${IMGUIIMPL_SOURCES})

file(GLOB IMGUI_FREETYPE_SOURCES
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/misc/freetype/imgui_freetype.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/misc/freetype/imgui_freetype.h)
source_group(src\\ImGui_FreeType FILES ${IMGUI_FREETYPE_SOURCES})

if (MSVC)
	set(ICON res/MSVCInfo.rc)
elseif (APPLE)
	set(ICON MACOSX_BUNDLE res/icon.icns)
	set_source_files_properties(res/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(${PROJECT}
	${ICON}
	${PROJECT_MAIN}
	${IMGUIIMPL_SOURCES}
	${PROJECT_SRC_RECURSE}
	${IMGUI_FREETYPE_SOURCES}
)

target_compile_definitions(${PROJECT} PUBLIC -DPROJECT_NAME=${PROJECT})
target_compile_definitions(${PROJECT} PRIVATE BUILD_GAIA_SHARED_LIBS)	
target_compile_definitions(${PROJECT} PRIVATE BUILD_CTOOLS_SHARED_LIBS)	
target_compile_definitions(${PROJECT} PRIVATE BUILD_IMGUI_PACK_SHARED_LIBS)
target_compile_definitions(${PROJECT} PRIVATE BUILD_LUMO_BACKEND_SHARED_LIBS)

if(USE_DEBUG_SANITIZER)
	target_compile_options(${PROJECT} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -static-libasan>)
	target_link_options(${PROJECT} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -static-libasan>)
	message("Address Sanitizer enabled for projet ${PROJECT}")
endif()

target_include_directories(${PROJECT} PRIVATE
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/3rdparty
	${LUMO_BACKEND_INCLUDE_DIRS}
	${Vulkan_INCLUDE_DIRS}
	${TINYXML2_INCLUDE_DIR}
)

target_link_libraries(${PROJECT}
	${LUMO_BACKEND_LIBRARIES}
	${TINYXML2_LIBRARIES}
)

set(${PROJECT}_Output_Name "${PROJECT}_${CMAKE_SYSTEM_NAME}_$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_Output_Name}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

## only for Debug
add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND BuildInc -prefix=LumoCodeGenerator 1000:10 $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/src/Headers/LumoCodeGeneratorBuild.h>
	DEPENDS ${PROJECT}
)
