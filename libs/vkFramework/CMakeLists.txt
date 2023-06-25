cmake_minimum_required(VERSION 3.1)

set(PROJECT vkFramework)

enable_language(C CXX)

set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")

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
	add_definitions(-DNOMINMAX) ## for avoid issue regarding std::max and std::min
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11") ## -std=gnu++0x
endif ()

find_package(Vulkan REQUIRED)
add_definitions(-DVULKAN)

## Vulkan HPP, default dispatcher
add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

## for prevent cast issues betwwen vk:: and VK if compiling a 32 bits version
add_definitions(-DVULKAN_HPP_TYPESAFE_CONVERSION=1) 

#if (BUILD_SHARED_LIBS)
#	add_library(${PROJECT} ${PROJECT_SRC_RECURSE})
#	target_compile_definitions(${PROJECT} INTERFACE BUILD_SHARED_LIBS)
#	set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
#	set_target_properties(${PROJECT} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#	target_link_libraries(${PROJECT} ${GLSLANG_LIBRARIES})
#else()
	add_library(${PROJECT} STATIC ${PROJECT_SRC_RECURSE})
#endif()

target_link_libraries(${PROJECT} 
	${Vulkan_LIBRARIES}
	${UTYPES_LIBRARIES}
	${GLSLANG_LIBRARIES}
	${TRACY_LIBRARIES} ## profiler
	${LODEPNG_LIBRARIES}
)
include_directories(
	${DEPENDS_PATH}
	${Vulkan_INCLUDE_DIRS}
)

set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)

set(VKFRAMEWORK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(VKFRAMEWORK_LIBRARIES ${PROJECT} PARENT_SCOPE)
set(VKFRAMEWORK_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
