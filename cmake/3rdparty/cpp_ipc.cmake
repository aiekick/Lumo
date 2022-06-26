add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/cpp_ipc EXCLUDE_FROM_ALL)

set_target_properties(ipc PROPERTIES FOLDER 3rdparty)
set_target_properties(ipc PROPERTIES LINK_FLAGS "/ignore:4244")

set(CPPIPC_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cpp_ipc/include)
set(CPPIPC_LIBRARIES ipc)
