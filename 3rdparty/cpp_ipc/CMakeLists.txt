cmake_minimum_required(VERSION 3.10)
project(cpp-ipc)

option(LIBIPC_BUILD_TESTS "Build all of libipc's own tests." OFF)
option(LIBIPC_BUILD_DEMOS "Build all of libipc's own demos." OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBIPC_PROJECT_DIR     ${PROJECT_SOURCE_DIR})

add_subdirectory(src)

if (LIBIPC_BUILD_TESTS)
    set(GOOGLETEST_VERSION 1.10.0)
    add_subdirectory(3rdparty/gtest)
    add_subdirectory(test)
endif()

if (LIBIPC_BUILD_DEMOS)
    add_subdirectory(demo/chat)
    add_subdirectory(demo/msg_que)
endif()

install(
  DIRECTORY "include/"
  DESTINATION "include"
)
