cmake_minimum_required(VERSION 3.1)

set(PROJECT Lumo)
set(PROJECT_VER 0.1.0)

enable_language(C CXX)

option(USE_CODE_GENERATOR "Enable the code generator" OFF)
option(USE_TRACY_PROFILER "Enable Tracy Profiler" OFF)
option(USE_TESTING_LUMO "Enable Tests" OFF)
option(USE_CONSOLE_APP "Enable Console" OFF)
option(USE_DEBUG_SANITIZER "Enable Address Sanitizer" OFF)

project(
	${PROJECT} 
	VERSION ${PROJECT_VER} 
	LANGUAGES CXX
)

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

set(FINAL_BIN_DIR ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_INSTALL_PREFIX "${FINAL_BIN_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}/debug")

set(USE_SHARED_LIBS ON CACHE BOOL "" FORCE)

if (USE_CODE_GENERATOR)
	add_definitions(-DUSE_CODE_GENERATOR)
endif()

if (USE_LUMO_TEST)
	enable_testing()
	add_definitions(-DTEST_ENABLED)
endif()

## for group smake targets in the dir cmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmakeTargets")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if (USE_RTX)
	add_definitions(-DUSE_RTX)
endif()

add_definitions(-DUSE_GLFW3)
add_definitions(-DUSE_STD_FILESYSTEM)
add_definitions(-DIMGUI_ENABLE_FREETYPE)
add_definitions(-DPROJECT_NAME="${PROJECT}")
add_definitions(-DVMA_VULKAN_VERSION=1002000)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")
add_definitions(-DUSE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS) ## for the resize imgui issue when we have child glfw windows
add_definitions(-DCUSTOM_LUMO_BACKEND_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomLumoBackendConfig.h")
add_definitions(-DCUSTOM_IMWIDGETS_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomImWidgetsConfig.h")
add_definitions(-DUSE_IMGUI="${CMAKE_SOURCE_DIR}/libs/Gaia/3rdparty/ImGuiPack/3rdparty/imgui/imgui.h") # for use ImGuiPack
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/src/Frontend/CustomImGuiFileDialogConfig.h")

set(USE_XML_CONFIG ON CACHE BOOL "" FORCE) # used by ImGuiPack by ex
if (USE_XML_CONFIG)
	add_definitions(-DUSE_XML_CONFIG)
endif()

if (USE_TRACY_PROFILER)
	add_definitions(-DPROFILER_INCLUDE="${CMAKE_SOURCE_DIR}/3rdparty/tracy/public/tracy/TracyVulkan.hpp")
	add_definitions(-DTRACY_ON_DEMAND)
	add_definitions(-DTRACY_ENABLE)
	set(TRACY_LIBRARIES TracyClient)
endif ()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()

include(cmake/3rdparty.cmake)
include(cmake/libs.cmake)

###########################################################
##### SEARCH FOR PLUGINS ##################################
###########################################################

set(PLUGINS_DIRECTORY ${CMAKE_SOURCE_DIR}/Plugins)
set(LOADED_LUMO_PLUGINS)
file(GLOB SUBDIRS RELATIVE ${PLUGINS_DIRECTORY} ${PLUGINS_DIRECTORY}/*)
FOREACH(SUBDIR ${SUBDIRS})
	if(IS_DIRECTORY ${PLUGINS_DIRECTORY}/${SUBDIR})
		if (EXISTS ${PLUGINS_DIRECTORY}/${SUBDIR}/CMakeLists.txt)
			add_subdirectory(${PLUGINS_DIRECTORY}/${SUBDIR})
		endif()
	endif()
ENDFOREACH()

###########################################################
###########################################################
###########################################################

set_target_properties(uninstall PROPERTIES FOLDER "cmakeTargets")

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

file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
source_group(main FILES ${PROJECT_MAIN})

file(GLOB_RECURSE PROJECT_CMAKE_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/cmake PREFIX cmake FILES ${PROJECT_CMAKE_FILES})

file(GLOB_RECURSE PROJECT_DOC_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/doc/*.*)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/doc PREFIX doc FILES ${PROJECT_DOC_FILES})

file(GLOB_RECURSE PROJECT_SRC_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_FILES})

file(GLOB README_MD 
	${CMAKE_CURRENT_SOURCE_DIR}/*.md)
source_group(help FILES ${README_MD})

add_executable(${PROJECT}
	${ICON}
	${README_MD}
	${PROJECT_MAIN}
	${PROJECT_DOC_FILES}
	${PROJECT_SRC_FILES}
	${PROJECT_CMAKE_FILES}
)

if (USE_PLUGINS)
	include(cmake/plugins_building.cmake)
endif()

if(USE_CONSOLE_APP)
	if (MSVC)
		target_link_options(${PROJECT} PRIVATE "/SUBSYSTEM:WINDOWS")
		target_link_options(${PROJECT} PRIVATE "/ENTRY:mainCRTStartup")
	endif()
endif()

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
	${PROJECT_PLUGINS}
	${LUMO_BACKEND_LIBRARIES}
	${TINYXML2_LIBRARIES}
)

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()

set(${PROJECT}_Output_Name "${PROJECT}_${CMAKE_SYSTEM_NAME}$<$<CONFIG:Debug>:_Debug>$<$<CONFIG:Release>:_Release>$<$<CONFIG:MinSizeRel>:_MinSizeRel>$<$<CONFIG:RelWithDebInfo>:_RelWithDebInfo>_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_Output_Name}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")

if (USE_TESTING_LUMO)
	enable_testing()
	add_definitions(-DTEST_ENABLED)
	#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

if (USE_TRACY_PROFILER)
	remove_definitions(-DTRACY_ENABLE)
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/tracy)
	
	set_target_properties(TracyClient PROPERTIES FOLDER 3rdparty/Shared/tracy)
	set_target_properties(capstone PROPERTIES FOLDER 3rdparty/Shared/tracy)

	set_target_properties(TracyClient PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
	set_target_properties(TracyClient PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
	set_target_properties(TracyClient PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
endif()

########################################################
############## FROCE BUILD OF PLUGINS ##################
########################################################

FOREACH(PLUGIN ${LOADED_LUMO_PLUGINS})
	add_dependencies(${PROJECT} ${PLUGIN})
ENDFOREACH()

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

## only for Debug
add_subdirectory(3rdparty/BuildInc)
set_target_properties(BuildInc PROPERTIES FOLDER Tools)

add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND BuildInc -prefix=${PROJECT} 1000000:10 $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/src/Headers/${PROJECT}Build.h>
	DEPENDS ${PROJECT}
)

########################################################
############# BUILD CODE GENERATOR TOOL ################
########################################################

if (USE_CODE_GENERATOR)
	add_subdirectory(Generator)
	set_target_properties(LumoCodeGenerator PROPERTIES FOLDER Lumo_Z_Apps)
endif()

#############################################################
### INSTALL #################################################
#############################################################

install(TARGETS 
	${PROJECT} 
	${ASSIMP_LIBRARIES}
	RUNTIME 
	DESTINATION /
	COMPONENT APP_BINARY)
##install(DIRECTORY 
##	${CMAKE_SOURCE_DIR}/projects 
##	DESTINATION /
##	COMPONENT APP_DIRECTORIES)
if (USE_PLUGINS)
	install(DIRECTORY 
		${FINAL_BIN_DIR}/plugins
		DESTINATION /
		COMPONENT APP_PLUGINS)
endif()
##install(FILES 
##	${CMAKE_SOURCE_DIR}/LICENSE
##	${CMAKE_SOURCE_DIR}/VERSION
##    DESTINATION /
##	COMPONENT APP_FILES)
##install(TARGETS ${ZLIB_LIBRARIES} RUNTIME DESTINATION / COMPONENT APP_DLL)

#############################################################
### VERSION #################################################
#############################################################

file(READ ${CMAKE_SOURCE_DIR}/src/Headers/LumoBuild.h BuildIdFile)
string(REGEX MATCH "#define ${PROJECT}_BuildNumber ([0-9]*)" _ ${BuildIdFile})
set(BuildNumber ${CMAKE_MATCH_1})
string(REGEX MATCH "#define ${PROJECT}_MinorNumber ([0-9]*)" _ ${BuildIdFile})
set(MinorNumber ${CMAKE_MATCH_1})
string(REGEX MATCH "#define ${PROJECT}_MajorNumber ([0-9]*)" _ ${BuildIdFile})
set(MajorNumber ${CMAKE_MATCH_1})
set(${PROJECT}_Version "v${MajorNumber}.${MinorNumber}.${BuildNumber}")
message("${PROJECT} Version to install : ${${PROJECT}_Version}")

#############################################################
### PACK ####################################################
## https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/Component-Install-With-CPack
#############################################################

if (USE_PACKAGE_TO_NSIS)
	set(CPACK_GENERATOR "NSIS")
else()
	set(CPACK_GENERATOR "ZIP")
endif()
set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_ZIP_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_PACKAGE_FILE_NAME "${${PROJECT}_Output_Name}_v${MajorNumber}.${MinorNumber}.${BuildNumber}")
set(CPACK_PACKAGE_VENDOR "FunParadigm")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${${PROJECT}_Output_Name} - offline shader editor")
set(CPACK_PACKAGE_VERSION "${MajorNumber}.${MinorNumber}.${BuildNumber}")
set(CPACK_PACKAGE_VERSION_MAJOR "${MajorNumber}")
set(CPACK_PACKAGE_VERSION_MINOR "${MinorNumber}")
set(CPACK_PACKAGE_VERSION_PATCH "${BuildNumber}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${FINAL_BIN_DIR}")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${CMAKE_PROJECT_NAME}")
##set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
##set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/VERSION")
SET(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_INSTALL_PREFIX}")
include(CPack)
