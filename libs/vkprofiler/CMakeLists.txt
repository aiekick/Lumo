cmake_minimum_required(VERSION 3.1)

set(PROJECT_LIB_NAME vkProfiler)
set(PROJECT_LIB_CASE_NAME VKPROFILER)

enable_language(C CXX)

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${PROJECT_SRC_RECURSE})

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
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11") ## -std=gnu++0x
endif ()

add_library(${PROJECT_LIB_NAME} STATIC ${PROJECT_SRC_RECURSE})

target_link_libraries(${PROJECT_LIB_NAME} PRIVATE 
	${Vulkan_LIBRARIES}
	${TRACY_LIBRARIES} ## profiler
)
include_directories(
	${DEPENDS_PATH}
)

set_target_properties(${PROJECT_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set(${PROJECT_LIB_CASE_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(${PROJECT_LIB_CASE_NAME}_LIBRARIES ${PROJECT_LIB_NAME} PARENT_SCOPE)
set(${PROJECT_LIB_CASE_NAME}_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
