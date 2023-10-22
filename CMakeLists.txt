cmake_minimum_required(VERSION 3.1)

set(PROJECT Lumo)
set(PROJECT_VER 0.1.0)

enable_language(C CXX)

option(USE_TRACY_PROFILER "Enable Tracy Profiler" OFF)
option(USE_TESTING_LUMO "Enable Tests" OFF)
option(USE_CONSOLE_APP "Enable Console" OFF)
option(USE_DEBUG_SANITIZER "Enable Address Sanitizer" OFF)
option(USE_PACKAGE_TO_NSIS_OR_ZIP "Package to NISIS Installer or ZIP" OFF)

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

set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}/debug")

set(CMAKE_INSTALL_PREFIX .)
set(CMAKE_INSTALL_BINDIR .)

set(USE_SHARED_LIBS ON CACHE BOOL "" FORCE)

if (USE_CODE_GENERATOR)
	add_definitions(-DUSE_CODE_GENERATOR)
endif()

if (USE_LUMO_TEST)
	enable_testing()
	add_definitions(-DTEST_ENABLED)
endif()

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

set(LLVM_USE_CRT_DEBUG MDd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MD CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "")

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()

include(cmake/3rdparty.cmake)
include(cmake/libs.cmake)

###########################################################
##### PLUGINS PREFIX ######################################
###########################################################

### each plugins must start by this

set(PLUGIN_PREFIX "Plugin")
add_definitions(-DPLUGIN_PREFIX="${PLUGIN_PREFIX}")

###########################################################
##### SEARCH FOR PLUGINS ##################################
###########################################################

set(LOADED_LUMO_PLUGINS) ### musty be fillerd by each pluigns in CMakeLists.txt with plugin name
set(PLUGINS_DIRECTORY ${CMAKE_SOURCE_DIR}/Plugins)
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17 -Wunused-parameter -Wno-unknown-pragmas")
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

file(GLOB_RECURSE GITHUB_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/.github/*.*)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/.github PREFIX github FILES ${GITHUB_FILES})

file(GLOB_RECURSE PROJECT_SRC_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_FILES})

file(GLOB README_MD 
	${CMAKE_CURRENT_SOURCE_DIR}/*.md)
source_group(Readmes/main FILES ${README_MD})

file(GLOB PLUGINS_README_MD 
	${CMAKE_CURRENT_SOURCE_DIR}/plugins/*.md)
source_group(Readmes/plugins FILES ${PLUGINS_README_MD})

add_executable(${PROJECT}
	${ICON}
	${README_MD}
	${GITHUB_FILES}
	${PROJECT_MAIN}
	${PLUGINS_README_MD}
	${PROJECT_DOC_FILES}
	${PROJECT_SRC_FILES}
	${PROJECT_CMAKE_FILES}
)

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
	${LUMO_BACKEND_LIBRARIES}
	${TINYXML2_LIBRARIES}
)

set(${PROJECT}_OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_OUTPUT_NAME}")
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
############## FORCE BUILD OF PLUGINS ##################
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

#############################################################
### VERSION #################################################
#############################################################

file(READ ${CMAKE_SOURCE_DIR}/src/Headers/${PROJECT}Build.h BuildIdFile)
string(REGEX MATCH "#define ${PROJECT}_BuildNumber ([0-9]*)" _ ${BuildIdFile})
set(BuildNumber ${CMAKE_MATCH_1})
string(REGEX MATCH "#define ${PROJECT}_MinorNumber ([0-9]*)" _ ${BuildIdFile})
set(MinorNumber ${CMAKE_MATCH_1})
string(REGEX MATCH "#define ${PROJECT}_MajorNumber ([0-9]*)" _ ${BuildIdFile})
set(MajorNumber ${CMAKE_MATCH_1})
set(${PROJECT}_Version "v${MajorNumber}_${MinorNumber}_${BuildNumber}")
message("${PROJECT} Version to install : ${${PROJECT}_Version}")

add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND echo "${PROJECT}_${CMAKE_SYSTEM_NAME}_${PROJECT_BUILD_CONFIG_NAME}_${ARCH}_v${MajorNumber}.${MinorNumber}.${BuildNumber}" > ${CMAKE_SOURCE_DIR}/VERSION
	DEPENDS ${PROJECT}
)

#############################################################
### BUILD CONFIG NAME #######################################
#############################################################

### will store the build config name in a file
### only during the building
add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND echo "$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>" > ${CMAKE_BINARY_DIR}/current_build_type
	DEPENDS ${PROJECT}
)

### outside of the building we read the last fiale content
### for get the current building config name
if (EXISTS ${CMAKE_BINARY_DIR}/current_build_type)
	file(READ ${CMAKE_BINARY_DIR}/current_build_type PROJECT_BUILD_CONFIG_NAME)
	string(STRIP ${PROJECT_BUILD_CONFIG_NAME} PROJECT_BUILD_CONFIG_NAME) ### removing end line wrote by echo command
	message("CURRENT BUILD CONFIG is : ${PROJECT_BUILD_CONFIG_NAME}")
endif()

#############################################################
### INSTALL #################################################
#############################################################

install(TARGETS ${PROJECT} RUNTIME DESTINATION . COMPONENT APP_BINARY)
install(DIRECTORY ${FINAL_BIN_DIR}/longlats	DESTINATION datas COMPONENT APP_LONGLATS)
install(DIRECTORY ${FINAL_BIN_DIR}/matcap DESTINATION datas COMPONENT APP_MATCAP)
install(DIRECTORY ${FINAL_BIN_DIR}/meshs DESTINATION datas COMPONENT APP_MESHS)
install(DIRECTORY ${FINAL_BIN_DIR}/shaders DESTINATION datas COMPONENT APP_SHADERS)
install(DIRECTORY ${FINAL_BIN_DIR}/skyboxes	DESTINATION datas COMPONENT APP_SKYBOXES)
install(DIRECTORY ${FINAL_BIN_DIR}/textures	DESTINATION datas COMPONENT APP_TEXTURES)
install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION .	COMPONENT APP_FILES)
install(FILES ${CMAKE_SOURCE_DIR}/VERSION DESTINATION .	COMPONENT APP_FILES OPTIONAL)
install(FILES ${CMAKE_SOURCE_DIR}/CHANGELOG DESTINATION . COMPONENT APP_FILES OPTIONAL)

### install is done in each plugins now
### FOREACH(PLUGIN ${LOADED_LUMO_PLUGINS})
### 	install(IMPORTED_RUNTIME_ARTIFACTS ${PLUGIN} DESTINATION plugins COMPONENT PLUGIN_BINARY_${PLUGIN})	
### ENDFOREACH()

#############################################################
### PACKAGE #################################################
## https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/Component-Install-With-CPack
#############################################################

set(PROJECT_PACKAGE_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_${PROJECT_BUILD_CONFIG_NAME}_${ARCH}_v${MajorNumber}.${MinorNumber}.${BuildNumber}")
set(PROJECT_SHORTCUT_NAME "${PROJECT}_v${MajorNumber}.${MinorNumber}.${BuildNumber}")
set(PROJECT_SHORTCUT_TARGET "${PROJECT}_${CMAKE_SYSTEM_NAME}_${PROJECT_BUILD_CONFIG_NAME}_${ARCH}")
set(ORGANIZATION_NAME "FunParadigm")

if (USE_PACKAGE_TO_NSIS_OR_ZIP)
	set(CPACK_GENERATOR "NSIS")
	set(CPACK_NSIS_MODIFY_PATH ON)
	set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_SHORTCUT_NAME})
	set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_SHORTCUT_NAME})
	if (WIN32)
		set(CPACK_PACKAGE_INSTALL_DIRECTORY "${ORGANIZATION_NAME}\\\\${PROJECT_PACKAGE_NAME}")
		set(CPACK_NSIS_CREATE_ICONS_EXTRA
			"CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${PROJECT_SHORTCUT_NAME}.lnk' '$INSTDIR\\\\${PROJECT_SHORTCUT_TARGET}.exe'"
		)
		set(CPACK_NSIS_DELETE_ICONS_EXTRA
			"Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${PROJECT_SHORTCUT_NAME}.lnk'"
		)
	endif()
else()
	set(CPACK_GENERATOR "ZIP")
endif()
set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_THREADS 8)
if (UNIX)
	set(CPACK_SET_DESTDIR ON)
endif()
set(CPACK_PACKAGE_VENDOR ${ORGANIZATION_NAME})
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_PACKAGE_VERSION_MAJOR "${MajorNumber}")
set(CPACK_PACKAGE_VERSION_MINOR "${MinorNumber}")
set(CPACK_PACKAGE_VERSION_PATCH "${BuildNumber}")
set(CPACK_PACKAGE_NAME "${PROJECT_PACKAGE_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_PACKAGE_NAME}")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${PROJECT}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_PACKAGE_NAME}")
SET(CPACK_OUTPUT_FILE_PREFIX "${FINAL_BIN_DIR}/packages")
set(CPACK_PACKAGE_VERSION "${MajorNumber}.${MinorNumber}.${BuildNumber}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${${PROJECT}_PACKAGE_NAME} - Nodes based mesh composer")

if (EXISTS ${CMAKE_SOURCE_DIR}/LICENSE)
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/VERSION)
	set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/VERSION")
endif()

include(CPack)
