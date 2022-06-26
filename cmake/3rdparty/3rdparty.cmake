if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(X11 REQUIRED)
  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

find_package(Vulkan REQUIRED)
add_definitions(-DVULKAN)

set(DEPENDS_PATH 
	${Vulkan_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glm
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glslang
	${CMAKE_CURRENT_SOURCE_DIR}/MainLibs
)

## contrib
include(cmake/3rdparty/googletest.cmake) ## the content will be added only if 'USE_TEST' is set
include(cmake/3rdparty/glslang.cmake)
##include(cmake/3rdparty/spirv_headers.cmake)
##include(cmake/3rdparty/spirv_tools.cmake)
include(cmake/3rdparty/glfw.cmake)
include(cmake/3rdparty/glm.cmake)
include(cmake/3rdparty/stb.cmake)
include(cmake/3rdparty/lodepng.cmake)
include(cmake/3rdparty/freetype.cmake)
include(cmake/3rdparty/imgui.cmake)
include(cmake/3rdparty/tinyxml2.cmake)
include(cmake/3rdparty/efsw.cmake)
include(cmake/3rdparty/imguicolortextedit.cmake)
include(cmake/3rdparty/cpp_ipc.cmake)
include(cmake/3rdparty/imguizmo.cmake)
include(cmake/3rdparty/imgui_node_editor.cmake)
include(cmake/3rdparty/assimp.cmake)

## aiekick
include(cmake/3rdparty/ctools.cmake)
include(cmake/3rdparty/imguifiledialog.cmake)

message("----------------------------------- Start Messages -----------------------------------")

include(cmake/3rdparty/vkframework.cmake)
include(cmake/3rdparty/vkprofiler.cmake)
include(cmake/3rdparty/utypes.cmake)
include(cmake/3rdparty/imwidgets.cmake)
include(cmake/3rdparty/fonticons.cmake)
include(cmake/3rdparty/common.cmake)

message("----------------------------------- End Messages -----------------------------------")
