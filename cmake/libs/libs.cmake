if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(X11 REQUIRED)
  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

find_package(Vulkan REQUIRED)
add_definitions(-DVULKAN)

## Vulkan HPP, default dispatcher
add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

## for prevent cast issues betwwen vk:: and VK if compiling a 32 bits version
add_definitions(-DVULKAN_HPP_TYPESAFE_CONVERSION=1) 

set(DEPENDS_PATH 
	${Vulkan_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glm
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glslang
	${CMAKE_CURRENT_SOURCE_DIR}/MainLibs
)

include(cmake/libs/ctools.cmake)
include(cmake/libs/imguifiledialog.cmake)

message("----------------------------------- Start Libs Messages -----------------------------------")

include(cmake/libs/vkframework.cmake)
include(cmake/libs/vkprofiler.cmake)
include(cmake/libs/utypes.cmake)
include(cmake/libs/imwidgets.cmake)
include(cmake/libs/fonticons.cmake)
include(cmake/libs/common.cmake)

message("----------------------------------- End Libs Messages -----------------------------------")
