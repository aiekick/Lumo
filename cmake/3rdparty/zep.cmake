add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)
##add_definitions(-DZEP_SINGLE_HEADER=1)

set(APP_ROOT ${CMAKE_CURRENT_LIST_DIR})
configure_file(${CMAKE_SOURCE_DIR}/cmake/3rdparty/zep_config_app.cmake ${CMAKE_BINARY_DIR}/config_app.h)

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zep/src)

set_target_properties(Zep PROPERTIES FOLDER 3rdparty)

set(ZEP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/zep/include)
set(ZEP_LIBRARIES Zep)
